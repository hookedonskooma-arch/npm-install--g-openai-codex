import AVFoundation
import Foundation

actor AudioDownloader {
    static let shared = AudioDownloader()

    func download(from urlString: String) async throws -> AVAudioPCMBuffer {
        guard let url = URL(string: urlString) else { throw DownloadError.badURL }
        let (localURL, _) = try await URLSession.shared.download(from: url)
        return try decode(fileURL: localURL)
    }

    private func decode(fileURL: URL) throws -> AVAudioPCMBuffer {
        let audioFile = try AVAudioFile(forReading: fileURL)
        let format = audioFile.processingFormat
        let frameCount = AVAudioFrameCount(audioFile.length)
        guard let buffer = AVAudioPCMBuffer(pcmFormat: format, frameCapacity: frameCount) else {
            throw DownloadError.bufferAllocationFailed
        }
        try audioFile.read(into: buffer)
        buffer.frameLength = frameCount
        return buffer
    }
}

enum DownloadError: LocalizedError {
    case badURL
    case bufferAllocationFailed

    var errorDescription: String? {
        switch self {
        case .badURL: return "Invalid audio URL from Suno"
        case .bufferAllocationFailed: return "Failed to allocate audio buffer"
        }
    }
}
