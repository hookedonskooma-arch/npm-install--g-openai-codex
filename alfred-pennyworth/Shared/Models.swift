import Foundation

enum GenerationMode: String, CaseIterable {
    case text = "Text Prompt"
    case cpp = "C++ Directive"
}

struct SongResult {
    let songId: String
    let audioURL: URL
    let title: String
}

struct GenerateRequest: Encodable {
    let prompt: String
    let style: String
    let title: String
    let mode: String
}

struct GenerateResponse: Decodable {
    let songId: String
}

struct StatusResponse: Decodable {
    let status: String
    let audio_url: String?
    let title: String?
    let image_url: String?
}
