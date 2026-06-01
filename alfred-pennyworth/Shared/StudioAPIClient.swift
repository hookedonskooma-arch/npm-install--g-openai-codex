import Foundation

actor StudioAPIClient {
    static let shared = StudioAPIClient()

    private var baseURL: String = UserDefaults.standard.string(forKey: "studioURL") ?? ""
    private var password: String = UserDefaults.standard.string(forKey: "studioPassword") ?? ""

    private let session: URLSession = {
        let config = URLSessionConfiguration.default
        config.httpCookieStorage = HTTPCookieStorage.shared
        config.httpShouldSetCookies = true
        return URLSession(configuration: config)
    }()

    func configure(baseURL: String, password: String) {
        self.baseURL = baseURL.trimmingCharacters(in: .whitespacesAndNewlines)
            .trimmingCharacters(in: CharacterSet(charactersIn: "/"))
        self.password = password
        UserDefaults.standard.set(self.baseURL, forKey: "studioURL")
        UserDefaults.standard.set(password, forKey: "studioPassword")
    }

    func generate(prompt: String, style: String, title: String, mode: GenerationMode) async throws -> String {
        try await login()
        let url = try validURL("/api/generate")
        var req = URLRequest(url: url)
        req.httpMethod = "POST"
        req.setValue("application/json", forHTTPHeaderField: "Content-Type")
        req.httpBody = try JSONEncoder().encode(
            GenerateRequest(prompt: prompt, style: style, title: title,
                            mode: mode == .cpp ? "cpp" : "text")
        )
        let (data, response) = try await session.data(for: req)
        guard (response as? HTTPURLResponse)?.statusCode == 200 else {
            throw APIError.serverError(String(data: data, encoding: .utf8) ?? "unknown")
        }
        return try JSONDecoder().decode(GenerateResponse.self, from: data).songId
    }

    func pollStatus(songId: String) async throws -> StatusResponse {
        let url = try validURL("/api/status/\(songId)")
        let (data, _) = try await session.data(from: url)
        return try JSONDecoder().decode(StatusResponse.self, from: data)
    }

    func saveToLibrary(id: String, title: String, prompt: String, style: String,
                       audioURL: String, imageURL: String?) async throws {
        let url = try validURL("/api/library")
        var req = URLRequest(url: url)
        req.httpMethod = "POST"
        req.setValue("application/json", forHTTPHeaderField: "Content-Type")
        var body: [String: String?] = [
            "id": id, "title": title, "prompt": prompt,
            "style": style, "audio_url": audioURL
        ]
        body["image_url"] = imageURL
        req.httpBody = try JSONEncoder().encode(body)
        _ = try? await session.data(for: req)
    }

    private func login() async throws {
        guard !password.isEmpty else { throw APIError.notConfigured }
        let url = try validURL("/api/login")
        var req = URLRequest(url: url)
        req.httpMethod = "POST"
        req.setValue("application/json", forHTTPHeaderField: "Content-Type")
        req.httpBody = try JSONEncoder().encode(["password": password])
        _ = try await session.data(for: req)
    }

    private func validURL(_ path: String) throws -> URL {
        guard !baseURL.isEmpty, let url = URL(string: baseURL + path) else {
            throw APIError.notConfigured
        }
        return url
    }
}

enum APIError: LocalizedError {
    case notConfigured
    case serverError(String)

    var errorDescription: String? {
        switch self {
        case .notConfigured:
            return "Studio URL not configured — tap the \u{2699} icon to set it."
        case .serverError(let msg):
            return "Server error: \(msg)"
        }
    }
}
