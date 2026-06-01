import type { Config } from "tailwindcss";

const config: Config = {
  content: [
    "./src/pages/**/*.{js,ts,jsx,tsx,mdx}",
    "./src/components/**/*.{js,ts,jsx,tsx,mdx}",
    "./src/app/**/*.{js,ts,jsx,tsx,mdx}",
  ],
  theme: {
    extend: {
      colors: {
        background: "#0a0a0a",
        card: "#111111",
        border: "#222222",
        accent: "#9b59b6",
        "accent-hover": "#8e44ad",
        text: "#e5e5e5",
        muted: "#666666",
      },
    },
  },
  plugins: [],
};

export default config;
