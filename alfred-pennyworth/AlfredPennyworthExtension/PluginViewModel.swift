import Foundation
import AVFoundation
import Combine

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
    @Published var showSettings: Bool = false
    @Published var studioURL: String = UserDefaults.standard.string(forKey: "studioURL") ?? ""
    @Published var studioPassword: String = UserDefaults.standard.string(forKey: "studioPassword") ?? ""

    weak var audioUnit: AlfredPennyworthAU?

    let stylePresets: [(String, String)] = [
        ("East Coast Underground", "east coast underground hip hop, boom bap, gritty, lo-fi, raw bars, jazz samples, 90s NYC"),
        ("Post-Hardcore Shoegaze", "post-hardcore, shoegaze, distorted guitars, ethereal vocals, reverb-soaked, dreamy, heavy, My Bloody Valentine"),
        ("Synthwave Noir", "synthwave, noir, 80s retrowave, dark synths, driving bassline, cinematic, neon-drenched"),
        ("Jazz Fusion", "jazz fusion, electric piano, complex chords, polyrhythm, Herbie Hancock, Miles Davis, improvisational"),
    ]

    func applyPreset(_ presetStyle: String) {
        self.style = presetStyle
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
        statusText = "Submitting to Suno..."

        Task {
            do {
                let songId = try await StudioAPIClient.shared.generate(
                    prompt: prompt,
                    style: style,
                    title: title,
                    mode: mode
                )
                statusText = "Generating... (up to 2 min)"

                var elapsed = 0
                while elapsed < 120 {
                    try await Task.sleep(nanoseconds: 3_000_000_000)
                    elapsed += 3
                    let status = try await StudioAPIClient.shared.pollStatus(songId: songId)
                    if status.status == "complete", let audioURL = status.audio_url {
                        statusText = "Downloading audio..."
                        let buffer = try await AudioDownloader.shared.download(from: audioURL)
                        audioUnit?.loadBuffer(buffer)
                        statusText = "Playing — record in Logic to capture"
                        songReady = true
                        loading = false
                        return
                    } else if status.status == "error" {
                        throw APIError.serverError("Suno generation failed")
                    }
                }
                throw APIError.serverError("Timed out waiting for Suno")
            } catch {
                errorText = error.localizedDescription
                statusText = ""
                loading = false
            }
        }
    }

    func replay() {
        audioUnit?.playheadFrame = 0
        audioUnit?.isPlaying = true
        statusText = "Playing — record in Logic to capture"
    }
}
