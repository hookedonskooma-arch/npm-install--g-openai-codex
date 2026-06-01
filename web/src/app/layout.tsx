import type { Metadata } from "next";
import "./globals.css";

export const metadata: Metadata = {
  title: "STUDIO",
  description: "Private music generation studio",
};

export default function RootLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  return (
    <html lang="en">
      <body style={{ minHeight: "100vh", backgroundColor: "#0a0a0a" }}>
        {children}
      </body>
    </html>
  );
}
