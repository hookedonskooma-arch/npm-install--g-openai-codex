import AudioToolbox
import AVFoundation
import CoreAudioKit

public class AlfredPennyworthAU: AUAudioUnit {
    private var outputBus: AUAudioUnitBus!
    private var _outputBusses: AUAudioUnitBusArray!

    // Shared audio buffer — written on background thread, read on render thread
    var pcmBuffer: AVAudioPCMBuffer?
    var playheadFrame: AVAudioFramePosition = 0
    var isPlaying: Bool = false

    public override init(componentDescription: AudioComponentDescription,
                         options: AudioComponentInstantiationOptions = []) throws {
        try super.init(componentDescription: componentDescription, options: options)

        let stereoFormat = AVAudioFormat(standardFormatWithSampleRate: 44100, channels: 2)!
        outputBus = try AUAudioUnitBus(format: stereoFormat)
        _outputBusses = AUAudioUnitBusArray(audioUnit: self, busType: .output, busses: [outputBus])
    }

    public override var outputBusses: AUAudioUnitBusArray { _outputBusses }

    public override func allocateRenderResources() throws {
        try super.allocateRenderResources()
    }

    public override func deallocateRenderResources() {
        super.deallocateRenderResources()
    }

    public override var canProcessInPlace: Bool { false }

    public override var internalRenderBlock: AUInternalRenderBlock {
        return { [weak self] _, _, frameCount, _, outputData, _, _ in
            guard let self = self,
                  self.isPlaying,
                  let buffer = self.pcmBuffer,
                  let channelData = buffer.floatChannelData else {
                // Output silence on all channels in the buffer list
                var bufferIndex = 0
                var mutableOutputData = outputData
                while mutableOutputData.pointee.mNumberBuffers > UInt32(bufferIndex) {
                    let audioBuffer = mutableOutputData.pointee.mBuffers
                    if let ptr = audioBuffer.mData {
                        memset(ptr, 0, Int(frameCount) * MemoryLayout<Float>.size)
                    }
                    bufferIndex += 1
                    mutableOutputData = UnsafeMutablePointer<AudioBufferList>(
                        OpaquePointer(mutableOutputData.advanced(by: 1))
                    )
                }
                return noErr
            }

            let remaining = Int(buffer.frameLength) - Int(self.playheadFrame)
            let framesToCopy = min(Int(frameCount), remaining)
            let channelCount = Int(min(buffer.format.channelCount, 2))

            for channel in 0..<channelCount {
                let src = channelData[channel].advanced(by: Int(self.playheadFrame))
                // Access the correct buffer in the AudioBufferList for this channel
                let audioBufferPtr: UnsafeMutableAudioBufferListPointer = UnsafeMutableAudioBufferListPointer(outputData)
                if channel < audioBufferPtr.count,
                   let dst = audioBufferPtr[channel].mData?.assumingMemoryBound(to: Float.self) {
                    memcpy(dst, src, framesToCopy * MemoryLayout<Float>.size)
                    if framesToCopy < Int(frameCount) {
                        memset(dst.advanced(by: framesToCopy), 0,
                               (Int(frameCount) - framesToCopy) * MemoryLayout<Float>.size)
                    }
                }
            }

            self.playheadFrame += AVAudioFramePosition(framesToCopy)
            if self.playheadFrame >= AVAudioFramePosition(buffer.frameLength) {
                self.isPlaying = false
                self.playheadFrame = 0
            }

            return noErr
        }
    }

    func loadBuffer(_ buffer: AVAudioPCMBuffer) {
        pcmBuffer = buffer
        playheadFrame = 0
        isPlaying = true
    }

    func stopPlayback() {
        isPlaying = false
        playheadFrame = 0
    }
}
