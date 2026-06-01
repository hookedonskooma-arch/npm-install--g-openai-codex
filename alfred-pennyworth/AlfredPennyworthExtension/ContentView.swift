import SwiftUI

struct ContentView: View {
    @ObservedObject var viewModel: PluginViewModel

    private let bgColor = Color(red: 0.04, green: 0.04, blue: 0.04)
    private let surfaceColor = Color(red: 0.09, green: 0.09, blue: 0.09)
    private let accentColor = Color(red: 0.608, green: 0.349, blue: 0.714)   // #9b59b6
    private let accentDim = Color(red: 0.35, green: 0.18, blue: 0.45)
    private let borderColor = Color(red: 0.22, green: 0.22, blue: 0.22)
    private let textPrimary = Color(red: 0.92, green: 0.92, blue: 0.92)
    private let textDim = Color(red: 0.45, green: 0.45, blue: 0.45)
    private let greenText = Color(red: 0.18, green: 0.85, blue: 0.40)

    var body: some View {
        ZStack {
            bgColor.ignoresSafeArea()

            ScrollView(.vertical, showsIndicators: false) {
                VStack(alignment: .leading, spacing: 12) {

                    // MARK: Header
                    HStack(alignment: .center) {
                        Text("ALFRED PENNYWORTH")
                            .font(.system(size: 14, weight: .bold, design: .monospaced))
                            .foregroundColor(accentColor)
                        Spacer()
                        Button(action: { viewModel.showSettings.toggle() }) {
                            Image(systemName: "gearshape.fill")
                                .foregroundColor(textDim)
                                .font(.system(size: 14))
                        }
                        .buttonStyle(.plain)
                        .help("Settings")
                    }
                    .padding(.bottom, 4)

                    Divider()
                        .background(borderColor)

                    // MARK: Mode picker
                    Picker("Mode", selection: $viewModel.mode) {
                        ForEach(GenerationMode.allCases, id: \.self) { m in
                            Text(m.rawValue).tag(m)
                        }
                    }
                    .pickerStyle(.segmented)
                    .labelsHidden()

                    // MARK: Prompt editor
                    VStack(alignment: .leading, spacing: 4) {
                        Text(viewModel.mode == .cpp ? "C++ DIRECTIVE" : "PROMPT")
                            .font(.system(size: 9, weight: .semibold, design: .monospaced))
                            .foregroundColor(textDim)
                            .tracking(1.2)

                        ZStack(alignment: .topLeading) {
                            RoundedRectangle(cornerRadius: 6)
                                .fill(surfaceColor)
                                .overlay(
                                    RoundedRectangle(cornerRadius: 6)
                                        .stroke(borderColor, lineWidth: 1)
                                )

                            if viewModel.prompt.isEmpty {
                                Text(viewModel.mode == .cpp
                                     ? "// e.g. generate_beat(style=\"boom bap\", bpm=90);"
                                     : "e.g. dark jazz ballad, smoky saxophone, rainy night...")
                                    .font(.system(size: 11, design: .monospaced))
                                    .foregroundColor(textDim)
                                    .padding(8)
                                    .allowsHitTesting(false)
                            }

                            TextEditor(text: $viewModel.prompt)
                                .font(.system(size: 11, design: .monospaced))
                                .foregroundColor(viewModel.mode == .cpp ? greenText : textPrimary)
                                .scrollContentBackground(.hidden)
                                .background(Color.clear)
                                .frame(height: 72)
                                .padding(4)
                        }
                        .frame(height: 80)
                    }

                    // MARK: Style presets
                    VStack(alignment: .leading, spacing: 4) {
                        Text("PRESETS")
                            .font(.system(size: 9, weight: .semibold, design: .monospaced))
                            .foregroundColor(textDim)
                            .tracking(1.2)

                        ScrollView(.horizontal, showsIndicators: false) {
                            HStack(spacing: 6) {
                                ForEach(viewModel.stylePresets, id: \.0) { name, style in
                                    Button(action: { viewModel.applyPreset(style) }) {
                                        Text(name)
                                            .font(.system(size: 10, design: .monospaced))
                                            .foregroundColor(accentColor)
                                            .padding(.horizontal, 8)
                                            .padding(.vertical, 4)
                                            .background(
                                                RoundedRectangle(cornerRadius: 4)
                                                    .fill(accentDim.opacity(0.25))
                                                    .overlay(
                                                        RoundedRectangle(cornerRadius: 4)
                                                            .stroke(accentDim, lineWidth: 1)
                                                    )
                                            )
                                    }
                                    .buttonStyle(.plain)
                                }
                            }
                        }
                    }

                    // MARK: Style/tags field
                    VStack(alignment: .leading, spacing: 4) {
                        Text("STYLE / TAGS")
                            .font(.system(size: 9, weight: .semibold, design: .monospaced))
                            .foregroundColor(textDim)
                            .tracking(1.2)

                        TextField("e.g. lo-fi, jazz, boom bap, 90bpm...", text: $viewModel.style)
                            .font(.system(size: 11, design: .monospaced))
                            .foregroundColor(textPrimary)
                            .textFieldStyle(.plain)
                            .padding(7)
                            .background(
                                RoundedRectangle(cornerRadius: 6)
                                    .fill(surfaceColor)
                                    .overlay(
                                        RoundedRectangle(cornerRadius: 6)
                                            .stroke(borderColor, lineWidth: 1)
                                    )
                            )
                    }

                    // MARK: Title field
                    VStack(alignment: .leading, spacing: 4) {
                        Text("TITLE (optional)")
                            .font(.system(size: 9, weight: .semibold, design: .monospaced))
                            .foregroundColor(textDim)
                            .tracking(1.2)

                        TextField("Song title...", text: $viewModel.title)
                            .font(.system(size: 11, design: .monospaced))
                            .foregroundColor(textPrimary)
                            .textFieldStyle(.plain)
                            .padding(7)
                            .background(
                                RoundedRectangle(cornerRadius: 6)
                                    .fill(surfaceColor)
                                    .overlay(
                                        RoundedRectangle(cornerRadius: 6)
                                            .stroke(borderColor, lineWidth: 1)
                                    )
                            )
                    }

                    // MARK: Generate button
                    Button(action: { viewModel.generate() }) {
                        HStack(spacing: 8) {
                            if viewModel.loading {
                                ProgressView()
                                    .progressViewStyle(.circular)
                                    .scaleEffect(0.65)
                                    .frame(width: 14, height: 14)
                            }
                            Text(viewModel.loading ? "GENERATING..." : "GENERATE")
                                .font(.system(size: 12, weight: .bold, design: .monospaced))
                                .tracking(1.5)
                        }
                        .foregroundColor(.white)
                        .frame(maxWidth: .infinity)
                        .padding(.vertical, 10)
                        .background(
                            RoundedRectangle(cornerRadius: 8)
                                .fill(viewModel.loading ? accentDim : accentColor)
                        )
                    }
                    .buttonStyle(.plain)
                    .disabled(viewModel.loading || viewModel.prompt.trimmingCharacters(in: .whitespaces).isEmpty)

                    // MARK: Status / error
                    if !viewModel.statusText.isEmpty {
                        HStack(spacing: 6) {
                            if viewModel.loading {
                                ProgressView()
                                    .progressViewStyle(.circular)
                                    .scaleEffect(0.55)
                                    .frame(width: 12, height: 12)
                            } else {
                                Image(systemName: "waveform")
                                    .foregroundColor(accentColor)
                                    .font(.system(size: 11))
                            }
                            Text(viewModel.statusText)
                                .font(.system(size: 11, design: .monospaced))
                                .foregroundColor(textDim)
                        }
                        .padding(.top, 2)
                    }

                    if !viewModel.errorText.isEmpty {
                        HStack(alignment: .top, spacing: 6) {
                            Image(systemName: "exclamationmark.triangle.fill")
                                .foregroundColor(.red)
                                .font(.system(size: 11))
                            Text(viewModel.errorText)
                                .font(.system(size: 11, design: .monospaced))
                                .foregroundColor(.red)
                                .fixedSize(horizontal: false, vertical: true)
                        }
                        .padding(8)
                        .background(
                            RoundedRectangle(cornerRadius: 6)
                                .fill(Color.red.opacity(0.10))
                                .overlay(
                                    RoundedRectangle(cornerRadius: 6)
                                        .stroke(Color.red.opacity(0.3), lineWidth: 1)
                                )
                        )
                    }

                    // MARK: Replay button
                    if viewModel.songReady {
                        Button(action: { viewModel.replay() }) {
                            HStack(spacing: 6) {
                                Image(systemName: "play.fill")
                                Text("REPLAY")
                                    .font(.system(size: 11, weight: .semibold, design: .monospaced))
                                    .tracking(1.2)
                            }
                            .foregroundColor(accentColor)
                            .frame(maxWidth: .infinity)
                            .padding(.vertical, 8)
                            .background(
                                RoundedRectangle(cornerRadius: 6)
                                    .fill(accentDim.opacity(0.20))
                                    .overlay(
                                        RoundedRectangle(cornerRadius: 6)
                                            .stroke(accentDim, lineWidth: 1)
                                    )
                            )
                        }
                        .buttonStyle(.plain)
                    }

                    Spacer(minLength: 8)
                }
                .padding(16)
            }
        }
        .frame(width: 480, height: 520)
        .sheet(isPresented: $viewModel.showSettings) {
            SettingsSheet(viewModel: viewModel)
        }
    }
}

// MARK: - Settings Sheet

struct SettingsSheet: View {
    @ObservedObject var viewModel: PluginViewModel

    private let bgColor = Color(red: 0.07, green: 0.07, blue: 0.07)
    private let surfaceColor = Color(red: 0.12, green: 0.12, blue: 0.12)
    private let accentColor = Color(red: 0.608, green: 0.349, blue: 0.714)
    private let borderColor = Color(red: 0.25, green: 0.25, blue: 0.25)
    private let textPrimary = Color(red: 0.92, green: 0.92, blue: 0.92)
    private let textDim = Color(red: 0.45, green: 0.45, blue: 0.45)

    var body: some View {
        VStack(alignment: .leading, spacing: 20) {
            HStack {
                Text("SETTINGS")
                    .font(.system(size: 14, weight: .bold, design: .monospaced))
                    .foregroundColor(accentColor)
                Spacer()
                Button(action: { viewModel.showSettings = false }) {
                    Image(systemName: "xmark.circle.fill")
                        .foregroundColor(textDim)
                        .font(.system(size: 16))
                }
                .buttonStyle(.plain)
            }

            Divider()
                .background(borderColor)

            VStack(alignment: .leading, spacing: 6) {
                Text("STUDIO URL")
                    .font(.system(size: 9, weight: .semibold, design: .monospaced))
                    .foregroundColor(textDim)
                    .tracking(1.2)

                TextField("https://your-studio-app.vercel.app", text: $viewModel.studioURL)
                    .font(.system(size: 12, design: .monospaced))
                    .foregroundColor(textPrimary)
                    .textFieldStyle(.plain)
                    .padding(8)
                    .background(
                        RoundedRectangle(cornerRadius: 6)
                            .fill(surfaceColor)
                            .overlay(
                                RoundedRectangle(cornerRadius: 6)
                                    .stroke(borderColor, lineWidth: 1)
                            )
                    )
            }

            VStack(alignment: .leading, spacing: 6) {
                Text("PASSWORD")
                    .font(.system(size: 9, weight: .semibold, design: .monospaced))
                    .foregroundColor(textDim)
                    .tracking(1.2)

                SecureField("Studio app password", text: $viewModel.studioPassword)
                    .font(.system(size: 12, design: .monospaced))
                    .foregroundColor(textPrimary)
                    .textFieldStyle(.plain)
                    .padding(8)
                    .background(
                        RoundedRectangle(cornerRadius: 6)
                            .fill(surfaceColor)
                            .overlay(
                                RoundedRectangle(cornerRadius: 6)
                                    .stroke(borderColor, lineWidth: 1)
                            )
                    )
            }

            Text("Your credentials are stored in macOS UserDefaults on this machine only.")
                .font(.system(size: 10, design: .monospaced))
                .foregroundColor(textDim)
                .fixedSize(horizontal: false, vertical: true)

            Spacer()

            Button(action: { viewModel.saveSettings() }) {
                Text("SAVE")
                    .font(.system(size: 12, weight: .bold, design: .monospaced))
                    .tracking(1.5)
                    .foregroundColor(.white)
                    .frame(maxWidth: .infinity)
                    .padding(.vertical, 10)
                    .background(RoundedRectangle(cornerRadius: 8).fill(accentColor))
            }
            .buttonStyle(.plain)
        }
        .padding(24)
        .frame(width: 380, height: 340)
        .background(bgColor)
    }
}
