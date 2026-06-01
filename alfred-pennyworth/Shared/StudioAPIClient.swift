import Foundation

actor StudioAPIClient {
    static let shared = StudioAPIClient()

    private var baseURL: String = UserDefaults.standard.string(forKey: "studioURL") ?? ""
    private var password: String = UserDefaults.standard.string(forKey: "studioPassword") ?? ""

    func configure(baseURL: String, password: String) {
        self.baseURL = baseURL
        UserDefaults.standard.set(baseURL, forKey: "studioURL")
        self.password = password
        UserDefaults.standard.set(password, forKey: "studioPassword")
    }

    // First authenticate (set session cookie), then generate
    func generate(prompt: String, style: String, title: String, mode: GenerationMode) async throws -> String {
        // Login to get session cookie
        try await login()

        let url = URL(string: "\(baseURL)/api/generate")!
        var req = URLRequest(url: url)
        req.httpMethod = "POST"
        req.setValue("application/json", forHTTPHeaderField: "Content-Type")
        let modeString = mode.rawValue == "C++ Directive" ? "cpp" : "text"
        let body = GenerateRequest(prompt: prompt, style: style, title: title, mode: modeString)
        req.httpBody = try JSONEncoder().encode(body)

        let (data, response) = try await URLSession.shared.data(for: req)
        guard (response as? HTTPURLResponse)?.statusCode == 200 else {
            throw APIError.serverError(String(data: data, encoding: .utf8) ?? "unknown")
        }
        let result = try JSONDecoder().decode(GenerateResponse.self, from: data)
        return result.songId
    }

    func pollStatus(songId: String) async throws -> StatusResponse {
        let url = URL(string: "\(baseURL)/api/status/\(songId)")!
        let (data, _) = try await URLSession.shared.data(from: url)
        return try JSONDecoder().decode(StatusResponse.self, from: data)
    }

    private func login() async throws {
        guard !password.isEmpty, !baseURL.isEmpty else { throw APIError.notConfigured }
        let url = URL(string: "\(baseURL)/api/login")!
        var req = URLRequest(url: url)
        req.httpMethod = "POST"
        req.setValue("application/json", forHTTPHeaderField: "Content-Type")
        req.httpBody = try JSONEncoder().encode(["password": password])
        _ = try await URLSession.shared.data(for: req)
    }
}

enum APIError: LocalizedError {
    case notConfigured
    case serverError(String)

    var errorDescription: String? {
        switch self {
        case .notConfigured: return "Studio URL and password not configured. Tap the \u{2699} icon."
        case .serverError(let msg): return "Server error: \(msg)"
        }
    }
}
