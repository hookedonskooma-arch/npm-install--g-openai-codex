import CoreAudioKit
import SwiftUI

public class AlfredPennyworthViewController: AUViewController, AUAudioUnitFactory {
    var audioUnit: AlfredPennyworthAU?
    private var viewModel: PluginViewModel?

    public override func viewDidLoad() {
        super.viewDidLoad()
        preferredContentSize = NSSize(width: 480, height: 520)
    }

    public func createAudioUnit(with componentDescription: AudioComponentDescription) throws -> AUAudioUnit {
        let au = try AlfredPennyworthAU(componentDescription: componentDescription, options: [])
        self.audioUnit = au

        DispatchQueue.main.async {
            self.setupUI(audioUnit: au)
        }
        return au
    }

    private func setupUI(audioUnit: AlfredPennyworthAU) {
        let vm = PluginViewModel()
        vm.audioUnit = audioUnit
        self.viewModel = vm
        let contentView = ContentView(viewModel: vm)
        let hostingView = NSHostingView(rootView: contentView)
        hostingView.frame = view.bounds
        hostingView.autoresizingMask = [.width, .height]
        view.addSubview(hostingView)
    }
}
