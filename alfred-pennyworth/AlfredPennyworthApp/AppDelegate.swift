import Cocoa

@main
class AppDelegate: NSObject, NSApplicationDelegate {
    var window: NSWindow?

    func applicationDidFinishLaunching(_ notification: Notification) {
        let vc = MainViewController()
        let window = NSWindow(contentViewController: vc)
        window.title = "Alfred Pennyworth"
        window.setContentSize(NSSize(width: 520, height: 320))
        window.center()
        window.makeKeyAndOrderFront(nil)
        self.window = window
    }

    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool { true }
}
