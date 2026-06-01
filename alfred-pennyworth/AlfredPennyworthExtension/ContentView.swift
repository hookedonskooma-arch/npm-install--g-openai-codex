import SwiftUI

private let BG      = Color(red: 0.04, green: 0.04, blue: 0.04)
private let SURFACE = Color(red: 0.09, green: 0.09, blue: 0.09)
private let ACCENT  = Color(red: 0.608, green: 0.349, blue: 0.714)
private let ACCDIM  = Color(red: 0.30, green: 0.15, blue: 0.40)
private let BORDER  = Color(red: 0.20, green: 0.20, blue: 0.20)
private let TEXT    = Color(red: 0.92, green: 0.92, blue: 0.92)
private let MUTED   = Color(red: 0.40, green: 0.40, blue: 0.40)
private let GREEN   = Color(red: 0.18, green: 0.85, blue: 0.40)

struct ContentView: View {
    @ObservedObject var viewModel: PluginViewModel

    var body: some View {
        ZStack {
            BG.ignoresSafeArea()
            ScrollView(.vertical, showsIndicators: false) {
                VStack(alignment: .leading, spacing: 10) {
                    HeaderRow(viewModel: viewModel)
                    Divider().background(BORDER)
                    ModeToggle(viewModel: viewModel)
                    PromptEditor(viewModel: viewModel)
                    PresetsRow(viewModel: viewModel)
                    FieldRow(label: "STYLE / TAGS", placeholder: "boom bap, lo-fi, 90bpm...", text: $viewModel.style)
                    FieldRow(label: "TITLE", placeholder: "Optional — leave blank for auto", text: $viewModel.title)
                    VolumeRow(viewModel: viewModel)
                    GenerateButton(viewModel: viewModel)
                    StatusRow(viewModel: viewModel)
                    if viewModel.songReady {
                        SongReadyRow(viewModel: viewModel)
                    }
                    Spacer(minLength: 8)
                }
                .padding(14)
            }
        }
        .frame(width: 480, height: 560)
        .sheet(isPresented: $viewModel.showSettings) {
            SettingsSheet(viewModel: viewModel)
        }
    }
}

// MARK: - Subviews

private struct HeaderRow: View {
    @ObservedObject var viewModel: PluginViewModel
    var body: some View {
        HStack {
            VStack(alignment: .leading, spacing: 1) {
                Text("ALFRED PENNYWORTH")
                    .font(.system(size: 13, weight: .bold, design: .monospaced))
                    .foregroundColor(ACCENT)
                Text("by STUDIO")
                    .font(.system(size: 9, design: .monospaced))
                    .foregroundColor(MUTED)
            }
            Spacer()
            Button { viewModel.showSettings.toggle() } label: {
                Image(systemName: "gearshape.fill")
                    .foregroundColor(MUTED)
                    .font(.system(size: 13))
            }
            .buttonStyle(.plain)
            .help("Settings — set Studio URL and password")
        }
    }
}

private struct ModeToggle: View {
    @ObservedObject var viewModel: PluginViewModel
    var body: some View {
        HStack(spacing: 0) {
            ForEach(GenerationMode.allCases, id: \.self) { m in
                Button { viewModel.mode = m } label: {
                    Text(m.rawValue)
                        .font(.system(size: 10, weight: .semibold, design: .monospaced))
                        .foregroundColor(viewModel.mode == m ? .white : MUTED)
                        .padding(.horizontal, 10)
                        .padding(.vertical, 5)
                        .background(viewModel.mode == m ? ACCENT : Color.clear)
                }
                .buttonStyle(.plain)
            }
        }
        .background(SURFACE)
        .overlay(Rectangle().stroke(BORDER, lineWidth: 1))
    }
}

private struct PromptEditor: View {
    @ObservedObject var viewModel: PluginViewModel
    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            Text(viewModel.mode == .cpp ? "C++ DIRECTIVE" : "PROMPT")
                .font(.system(size: 9, weight: .semibold, design: .monospaced))
                .foregroundColor(MUTED)
                .tracking(1.2)

            ZStack(alignment: .topLeading) {
                SURFACE
                if viewModel.prompt.isEmpty {
                    Text(viewModel.mode == .cpp
                         ? "// e.g.\nstruct Track { int bpm = 90;\n  void verse() { flow(aggressive); } };"
                         : "Describe your song — no character limit. Mood, lyrics, structure, influences...")
                        .font(.system(size: 11, design: .monospaced))
                        .foregroundColor(MUTED)
                        .padding(8)
                        .allowsHitTesting(false)
                }
                TextEditor(text: $viewModel.prompt)
                    .font(.system(size: 11, design: .monospaced))
                    .foregroundColor(viewModel.mode == .cpp ? GREEN : TEXT)
                    .scrollContentBackground(.hidden)
                    .background(Color.clear)
                    .frame(minHeight: 120)
                    .padding(4)
            }
            .frame(minHeight: 136)
            .overlay(Rectangle().stroke(BORDER, lineWidth: 1))

            if viewModel.mode == .cpp {
                Text("OpenAI will interpret the C++ as musical directives before sending to Suno.")
                    .font(.system(size: 9, design: .monospaced))
                    .foregroundColor(MUTED)
            }
        }
    }
}

private struct PresetsRow: View {
    @ObservedObject var viewModel: PluginViewModel
    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            Text("PRESETS")
                .font(.system(size: 9, weight: .semibold, design: .monospaced))
                .foregroundColor(MUTED)
                .tracking(1.2)
            ScrollView(.horizontal, showsIndicators: false) {
                HStack(spacing: 5) {
                    ForEach(viewModel.stylePresets, id: \.0) { name, style in
                        Button { viewModel.applyPreset(style) } label: {
                            Text(name)
                                .font(.system(size: 10, design: .monospaced))
                                .foregroundColor(ACCENT)
                                .padding(.horizontal, 8)
                                .padding(.vertical, 4)
                                .background(ACCDIM.opacity(0.3))
                                .overlay(Rectangle().stroke(ACCDIM, lineWidth: 1))
                        }
                        .buttonStyle(.plain)
                    }
                }
            }
        }
    }
}

private struct FieldRow: View {
    let label: String
    let placeholder: String
    @Binding var text: String
    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            Text(label)
                .font(.system(size: 9, weight: .semibold, design: .monospaced))
                .foregroundColor(MUTED)
                .tracking(1.2)
            TextField(placeholder, text: $text)
                .font(.system(size: 11, design: .monospaced))
                .foregroundColor(TEXT)
                .textFieldStyle(.plain)
                .padding(7)
                .background(SURFACE)
                .overlay(Rectangle().stroke(BORDER, lineWidth: 1))
        }
    }
}

private struct VolumeRow: View {
    @ObservedObject var viewModel: PluginViewModel
    var body: some View {
        HStack(spacing: 8) {
            Text("VOL")
                .font(.system(size: 9, weight: .semibold, design: .monospaced))
                .foregroundColor(MUTED)
                .tracking(1.2)
                .frame(width: 28, alignment: .leading)
            Slider(value: $viewModel.volume, in: 0...1)
                .accentColor(ACCENT)
            Text(String(format: "%.0f%%", viewModel.volume * 100))
                .font(.system(size: 9, design: .monospaced))
                .foregroundColor(MUTED)
                .frame(width: 30, alignment: .trailing)
        }
    }
}

private struct GenerateButton: View {
    @ObservedObject var viewModel: PluginViewModel
    var body: some View {
        Button { viewModel.generate() } label: {
            HStack(spacing: 8) {
                if viewModel.loading {
                    ProgressView()
                        .progressViewStyle(.circular)
                        .scaleEffect(0.6)
                        .frame(width: 14, height: 14)
                }
                Text(viewModel.loading ? "GENERATING..." : "GENERATE")
                    .font(.system(size: 12, weight: .bold, design: .monospaced))
                    .tracking(1.5)
            }
            .foregroundColor(.white)
            .frame(maxWidth: .infinity)
            .padding(.vertical, 10)
            .background(viewModel.loading ? ACCDIM : ACCENT)
        }
        .buttonStyle(.plain)
        .disabled(viewModel.loading || viewModel.prompt.trimmingCharacters(in: .whitespaces).isEmpty)
    }
}

private struct StatusRow: View {
    @ObservedObject var viewModel: PluginViewModel
    var body: some View {
        Group {
            if !viewModel.statusText.isEmpty {
                HStack(spacing: 6) {
                    if viewModel.loading {
                        ProgressView().progressViewStyle(.circular).scaleEffect(0.5).frame(width: 12, height: 12)
                    } else {
                        Image(systemName: "waveform").foregroundColor(ACCENT).font(.system(size: 10))
                    }
                    Text(viewModel.statusText)
                        .font(.system(size: 10, design: .monospaced))
                        .foregroundColor(MUTED)
                }
            }
            if !viewModel.errorText.isEmpty {
                HStack(alignment: .top, spacing: 6) {
                    Image(systemName: "exclamationmark.triangle.fill")
                        .foregroundColor(.red).font(.system(size: 10))
                    Text(viewModel.errorText)
                        .font(.system(size: 10, design: .monospaced))
                        .foregroundColor(.red)
                        .fixedSize(horizontal: false, vertical: true)
                }
                .padding(8)
                .background(Color.red.opacity(0.08))
                .overlay(Rectangle().stroke(Color.red.opacity(0.25), lineWidth: 1))
            }
        }
    }
}

private struct SongReadyRow: View {
    @ObservedObject var viewModel: PluginViewModel
    @State private var saved = false

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            if !viewModel.songTitle.isEmpty {
                Text(viewModel.songTitle.uppercased())
                    .font(.system(size: 10, weight: .semibold, design: .monospaced))
                    .foregroundColor(ACCENT)
                    .lineLimit(1)
                    .truncationMode(.tail)
            }
            HStack(spacing: 6) {
                Button { viewModel.replay() } label: {
                    HStack(spacing: 5) {
                        Image(systemName: "play.fill")
                        Text("REPLAY")
                            .font(.system(size: 10, weight: .semibold, design: .monospaced))
                            .tracking(1.2)
                    }
                    .foregroundColor(ACCENT)
                    .frame(maxWidth: .infinity)
                    .padding(.vertical, 7)
                    .background(ACCDIM.opacity(0.2))
                    .overlay(Rectangle().stroke(ACCDIM, lineWidth: 1))
                }
                .buttonStyle(.plain)

                Button {
                    viewModel.saveToLibrary()
                    saved = true
                } label: {
                    HStack(spacing: 5) {
                        Image(systemName: saved ? "checkmark" : "square.and.arrow.down")
                        Text(saved ? "SAVED" : "SAVE")
                            .font(.system(size: 10, weight: .semibold, design: .monospaced))
                            .tracking(1.2)
                    }
                    .foregroundColor(saved ? MUTED : ACCENT)
                    .frame(maxWidth: .infinity)
                    .padding(.vertical, 7)
                    .background(saved ? Color.clear : ACCDIM.opacity(0.2))
                    .overlay(Rectangle().stroke(saved ? BORDER : ACCDIM, lineWidth: 1))
                }
                .buttonStyle(.plain)
                .disabled(saved)
            }
        }
    }
}

// MARK: - Settings Sheet

struct SettingsSheet: View {
    @ObservedObject var viewModel: PluginViewModel

    var body: some View {
        VStack(alignment: .leading, spacing: 18) {
            HStack {
                Text("SETTINGS")
                    .font(.system(size: 13, weight: .bold, design: .monospaced))
                    .foregroundColor(ACCENT)
                Spacer()
                Button { viewModel.showSettings = false } label: {
                    Image(systemName: "xmark.circle.fill")
                        .foregroundColor(MUTED).font(.system(size: 15))
                }
                .buttonStyle(.plain)
            }

            Divider().background(BORDER)

            settingField(label: "STUDIO URL", placeholder: "https://your-app.vercel.app",
                         text: $viewModel.studioURL)
            settingSecure(label: "PASSWORD", placeholder: "Your private Studio password",
                          text: $viewModel.studioPassword)

            Text("Credentials are stored in macOS UserDefaults on this machine only and are never transmitted except to your own Studio URL.")
                .font(.system(size: 9, design: .monospaced))
                .foregroundColor(MUTED)
                .fixedSize(horizontal: false, vertical: true)

            Spacer()

            Button { viewModel.saveSettings() } label: {
                Text("SAVE")
                    .font(.system(size: 12, weight: .bold, design: .monospaced))
                    .tracking(1.5)
                    .foregroundColor(.white)
                    .frame(maxWidth: .infinity)
                    .padding(.vertical, 10)
                    .background(ACCENT)
            }
            .buttonStyle(.plain)
        }
        .padding(22)
        .frame(width: 380, height: 320)
        .background(Color(red: 0.06, green: 0.06, blue: 0.06))
    }

    @ViewBuilder
    private func settingField(label: String, placeholder: String, text: Binding<String>) -> some View {
        VStack(alignment: .leading, spacing: 4) {
            Text(label)
                .font(.system(size: 9, weight: .semibold, design: .monospaced))
                .foregroundColor(MUTED).tracking(1.2)
            TextField(placeholder, text: text)
                .font(.system(size: 11, design: .monospaced))
                .foregroundColor(TEXT).textFieldStyle(.plain)
                .padding(7)
                .background(SURFACE)
                .overlay(Rectangle().stroke(BORDER, lineWidth: 1))
        }
    }

    @ViewBuilder
    private func settingSecure(label: String, placeholder: String, text: Binding<String>) -> some View {
        VStack(alignment: .leading, spacing: 4) {
            Text(label)
                .font(.system(size: 9, weight: .semibold, design: .monospaced))
                .foregroundColor(MUTED).tracking(1.2)
            SecureField(placeholder, text: text)
                .font(.system(size: 11, design: .monospaced))
                .foregroundColor(TEXT).textFieldStyle(.plain)
                .padding(7)
                .background(SURFACE)
                .overlay(Rectangle().stroke(BORDER, lineWidth: 1))
        }
    }
}
