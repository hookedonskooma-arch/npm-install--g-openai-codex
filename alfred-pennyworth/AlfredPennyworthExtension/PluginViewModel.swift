import AVFoundation
import Combine
import Foundation

@MainActor
class PluginViewModel: ObservableObject {
    @Published var prompt: String = ""
    @Published var style: String = ""
    @Published var title: String = ""
    @Published var mode: GenerationMode = .text
    @Published var loading: Bool = false
    @Published var statusText: String = ""
    @Published var errorText: String = ""
    @Published var songReady: Bool = false
    @Published var songTitle: String = ""
    @Published var songId: String = ""
    @Published var audioURL: String = ""
    @Published var showSettings: Bool = false
    @Published var studioURL: String = UserDefaults.standard.string(forKey: "studioURL") ?? ""
    @Published var studioPassword: String = UserDefaults.standard.string(forKey: "studioPassword") ?? ""
    @Published var volume: Double = 1.0 {
        didSet { audioUnit?.gain = Float(volume) }
    }

    weak var audioUnit: AlfredPennyworthAU?

    let stylePresets: [(String, String)] = [
        ("East Coast Underground",
         "east coast underground hip hop, boom bap, gritty, lo-fi, raw bars, jazz samples, 90s NYC"),
        ("Post-Hardcore Shoegaze",
         "post-hardcore, shoegaze, distorted guitars, ethereal vocals, reverb-soaked, dreamy, heavy, My Bloody Valentine"),
        ("Synthwave Noir",
         "synthwave, noir, 80s retrowave, dark synths, driving bassline, cinematic, neon-drenched"),
        ("Jazz Fusion",
         "jazz fusion, electric piano, complex chords, polyrhythm, Herbie Hancock, Miles Davis, improvisational"),
    ]

    func applyPreset(_ presetStyle: String) {
        style = presetStyle
    }

    func saveSettings() {
        Task {
            await StudioAPIClient.shared.configure(baseURL: studioURL, password: studioPassword)
        }
        showSettings = false
    }

    func generate() {
        guard !prompt.trimmingCharacters(in: .whitespaces).isEmpty else { return }
        loading = true
        errorText = ""
        songReady = false
        songTitle = ""
        songId = ""
        audioURL = ""
        statusText = "Submitting..."

        Task {
            do {
                let id = try await StudioAPIClient.shared.generate(
                    prompt: prompt, style: style, title: title, mode: mode
                )
                songId = id
                statusText = "Generating — up to 2 min"

                for _ in 0..<40 {
                    try await Task.sleep(nanoseconds: 3_000_000_000)
                    let status = try await StudioAPIClient.shared.pollStatus(songId: id)
                    if status.status == "complete", let url = status.audio_url {
                        statusText = "Downloading..."
                        let buffer = try await AudioDownloader.shared.download(from: url)
                        audioUnit?.loadBuffer(buffer)
                        songTitle = status.title ?? title.isEmpty ? "Untitled" : title
                        audioURL = url
                        songReady = true
                        statusText = "Ready — arm track in Logic and hit record"
                        loading = false
                        return
                    } else if status.status == "error" {
                        throw APIError.serverError("Suno generation failed")
                    }
                }
                throw APIError.serverError("Timed out waiting for Suno (2 min)")
            } catch {
                errorText = error.localizedDescription
                statusText = ""
                loading = false
            }
        }
    }

    func replay() {
        guard let au = audioUnit else { return }
        au.playheadFrame = 0
        au.isPlaying = true
        statusText = "Playing — arm track in Logic and hit record"
    }

    func saveToLibrary() {
        guard !songId.isEmpty, !audioURL.isEmpty else { return }
        Task {
            try? await StudioAPIClient.shared.saveToLibrary(
                id: songId,
                title: songTitle.isEmpty ? "Untitled" : songTitle,
                prompt: prompt,
                style: style,
                audioURL: audioURL,
                imageURL: nil
            )
        }
    }
}
