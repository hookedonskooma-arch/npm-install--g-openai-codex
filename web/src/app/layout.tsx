import type { Metadata, Viewport } from "next";
import "./globals.css";
import { ServiceWorkerRegister } from "@/components/ServiceWorkerRegister";

export const metadata: Metadata = {
  title: "STUDIO",
  description: "Private music generation studio",
  manifest: "/manifest.json",
  appleWebApp: {
    capable: true,
    title: "STUDIO",
    statusBarStyle: "black-translucent",
  },
  icons: {
    apple: "/icons/apple-touch-icon.png",
    icon: "/icons/icon-192.png",
  },
};

export const viewport: Viewport = {
  themeColor: "#9b59b6",
  width: "device-width",
  initialScale: 1,
  viewportFit: "cover",
};

export default function RootLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  return (
    <html lang="en">
      <body style={{ minHeight: "100vh", backgroundColor: "#0a0a0a" }}>
        <ServiceWorkerRegister />
        {children}
      </body>
    </html>
  );
}
