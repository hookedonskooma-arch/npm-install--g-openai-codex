import Cocoa

class MainViewController: NSViewController {

    override func loadView() {
        view = NSView(frame: NSRect(x: 0, y: 0, width: 520, height: 320))
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        setupUI()
    }

    private func setupUI() {
        // Dark background
        view.wantsLayer = true
        view.layer?.backgroundColor = NSColor(calibratedRed: 0.04, green: 0.04, blue: 0.04, alpha: 1.0).cgColor

        // Title label
        let titleLabel = NSTextField(labelWithString: "ALFRED PENNYWORTH")
        titleLabel.font = NSFont.monospacedSystemFont(ofSize: 22, weight: .bold)
        titleLabel.textColor = NSColor(calibratedRed: 0.608, green: 0.349, blue: 0.714, alpha: 1.0) // #9b59b6
        titleLabel.alignment = .center
        titleLabel.translatesAutoresizingMaskIntoConstraints = false
        view.addSubview(titleLabel)

        // Subtitle / tagline
        let subtitleLabel = NSTextField(labelWithString: "AI Music Generator for Logic Pro")
        subtitleLabel.font = NSFont.monospacedSystemFont(ofSize: 12, weight: .regular)
        subtitleLabel.textColor = NSColor.secondaryLabelColor
        subtitleLabel.alignment = .center
        subtitleLabel.translatesAutoresizingMaskIntoConstraints = false
        view.addSubview(subtitleLabel)

        // Divider
        let divider = NSBox()
        divider.boxType = .separator
        divider.translatesAutoresizingMaskIntoConstraints = false
        view.addSubview(divider)

        // Instructions
        let instructionsText = """
To use Alfred Pennyworth in Logic Pro:

  1. In Logic Pro, create a new Software Instrument track.
  2. Click the instrument slot and choose "AU Generators".
  3. Select Alfred Pennyworth → "Alfred Pennyworth: Studio".
  4. In the plugin window, click ⚙ to enter your Studio URL and password.
  5. Type a prompt and click Generate.
  6. Press Record in Logic to capture the generated audio.

Note: Make sure the Alfred Pennyworth app has been built and installed
before opening Logic Pro.
"""
        let instructionsLabel = NSTextField(wrappingLabelWithString: instructionsText)
        instructionsLabel.font = NSFont.monospacedSystemFont(ofSize: 11, weight: .regular)
        instructionsLabel.textColor = NSColor.labelColor
        instructionsLabel.translatesAutoresizingMaskIntoConstraints = false
        view.addSubview(instructionsLabel)

        // Note label
        let noteLabel = NSTextField(labelWithString: "Studio URL can be set inside the plugin window in Logic Pro.")
        noteLabel.font = NSFont.monospacedSystemFont(ofSize: 10, weight: .regular)
        noteLabel.textColor = NSColor.tertiaryLabelColor
        noteLabel.alignment = .center
        noteLabel.translatesAutoresizingMaskIntoConstraints = false
        view.addSubview(noteLabel)

        NSLayoutConstraint.activate([
            titleLabel.topAnchor.constraint(equalTo: view.topAnchor, constant: 32),
            titleLabel.centerXAnchor.constraint(equalTo: view.centerXAnchor),

            subtitleLabel.topAnchor.constraint(equalTo: titleLabel.bottomAnchor, constant: 6),
            subtitleLabel.centerXAnchor.constraint(equalTo: view.centerXAnchor),

            divider.topAnchor.constraint(equalTo: subtitleLabel.bottomAnchor, constant: 16),
            divider.leadingAnchor.constraint(equalTo: view.leadingAnchor, constant: 24),
            divider.trailingAnchor.constraint(equalTo: view.trailingAnchor, constant: -24),

            instructionsLabel.topAnchor.constraint(equalTo: divider.bottomAnchor, constant: 16),
            instructionsLabel.leadingAnchor.constraint(equalTo: view.leadingAnchor, constant: 28),
            instructionsLabel.trailingAnchor.constraint(equalTo: view.trailingAnchor, constant: -28),

            noteLabel.bottomAnchor.constraint(equalTo: view.bottomAnchor, constant: -16),
            noteLabel.centerXAnchor.constraint(equalTo: view.centerXAnchor),
        ])
    }
}
