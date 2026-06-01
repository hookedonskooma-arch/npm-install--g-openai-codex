import Accelerate
import AudioToolbox
import AVFoundation
import CoreAudioKit

public class AlfredPennyworthAU: AUAudioUnit {
    private var outputBus: AUAudioUnitBus!
    private var _outputBusses: AUAudioUnitBusArray!

    // Written on main thread after download; read on render thread.
    // Buffer pointer is stable once set (we hold the strong ref below).
    private var currentBuffer: AVAudioPCMBuffer?
    var playheadFrame: AVAudioFramePosition = 0
    var isPlaying: Bool = false
    var gain: Float = 1.0

    public override init(
        componentDescription: AudioComponentDescription,
        options: AudioComponentInstantiationOptions = []
    ) throws {
        try super.init(componentDescription: componentDescription, options: options)
        let stereo = AVAudioFormat(standardFormatWithSampleRate: 44100, channels: 2)!
        outputBus = try AUAudioUnitBus(format: stereo)
        outputBus.maximumChannelCount = 2
        _outputBusses = AUAudioUnitBusArray(audioUnit: self, busType: .output, busses: [outputBus])
    }

    public override var outputBusses: AUAudioUnitBusArray { _outputBusses }
    public override var canProcessInPlace: Bool { false }

    public override func allocateRenderResources() throws {
        try super.allocateRenderResources()
        // If a buffer was loaded before resources were allocated, resample it now
        // to match the negotiated host format.
        if let buf = currentBuffer, buf.format != outputBus.format {
            currentBuffer = resample(buf, to: outputBus.format) ?? buf
        }
    }

    public override var internalRenderBlock: AUInternalRenderBlock {
        return { [weak self] _, _, frameCount, _, outputData, _, _ in
            let out = UnsafeMutableAudioBufferListPointer(outputData)

            guard let self,
                  self.isPlaying,
                  let buffer = self.currentBuffer,
                  let channelData = buffer.floatChannelData
            else {
                for i in 0..<out.count {
                    if let ptr = out[i].mData {
                        memset(ptr, 0, Int(frameCount) * MemoryLayout<Float>.size)
                    }
                }
                return noErr
            }

            let gain = self.gain
            let remaining = max(0, Int(buffer.frameLength) - Int(self.playheadFrame))
            let toCopy = min(Int(frameCount), remaining)
            let srcChannels = Int(buffer.format.channelCount)
            let dstChannels = out.count

            for ch in 0..<dstChannels {
                guard let dst = out[ch].mData?.assumingMemoryBound(to: Float.self) else { continue }
                let srcCh = min(ch, srcChannels - 1)   // upmix mono → stereo
                if toCopy > 0 {
                    let src = channelData[srcCh].advanced(by: Int(self.playheadFrame))
                    if gain == 1.0 {
                        memcpy(dst, src, toCopy * MemoryLayout<Float>.size)
                    } else {
                        vDSP_vsmul(src, 1, [gain], dst, 1, vDSP_Length(toCopy))
                    }
                }
                if toCopy < Int(frameCount) {
                    memset(dst.advanced(by: toCopy), 0,
                           (Int(frameCount) - toCopy) * MemoryLayout<Float>.size)
                }
            }

            self.playheadFrame += AVAudioFramePosition(toCopy)
            if self.playheadFrame >= AVAudioFramePosition(buffer.frameLength) {
                self.isPlaying = false
                self.playheadFrame = 0
            }

            return noErr
        }
    }

    func loadBuffer(_ buffer: AVAudioPCMBuffer) {
        let targetFormat = outputBus.format
        let final = (buffer.format == targetFormat) ? buffer : (resample(buffer, to: targetFormat) ?? buffer)
        currentBuffer = final
        playheadFrame = 0
        isPlaying = true
    }

    func stopPlayback() {
        isPlaying = false
        playheadFrame = 0
    }

    // MARK: - Resampling

    private func resample(_ buffer: AVAudioPCMBuffer, to format: AVAudioFormat) -> AVAudioPCMBuffer? {
        guard let converter = AVAudioConverter(from: buffer.format, to: format) else { return nil }
        let ratio = format.sampleRate / buffer.format.sampleRate
        let capacity = AVAudioFrameCount(Double(buffer.frameLength) * ratio) + 1
        guard let output = AVAudioPCMBuffer(pcmFormat: format, frameCapacity: capacity) else { return nil }

        var inputConsumed = false
        var convError: NSError?
        converter.convert(to: output, error: &convError) { _, outStatus in
            if inputConsumed {
                outStatus.pointee = .endOfStream
                return nil
            }
            outStatus.pointee = .haveData
            inputConsumed = true
            return buffer
        }
        return convError == nil ? output : nil
    }
}
