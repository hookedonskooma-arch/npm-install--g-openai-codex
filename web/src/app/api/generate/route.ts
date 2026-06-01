import { NextRequest, NextResponse } from "next/server";
import OpenAI from "openai";
import { generateSong } from "@/lib/suno";

export async function POST(request: NextRequest) {
  try {
    const { prompt, style, title, mode } = await request.json();

    if (!prompt || typeof prompt !== "string") {
      return NextResponse.json({ error: "prompt is required" }, { status: 400 });
    }

    let finalPrompt = prompt;

    if (mode === "cpp") {
      const openai = new OpenAI({ apiKey: process.env.OPENAI_API_KEY });
      const completion = await openai.chat.completions.create({
        model: "gpt-4o",
        messages: [
          {
            role: "system",
            content:
              "You are a music director. Interpret the following C++ code as musical directives. Map function names to song sections, loops to repeated patterns, variable names to sonic elements, comments to lyrical themes or production notes. Output a rich, detailed music generation prompt for Suno in 2-3 paragraphs.",
          },
          {
            role: "user",
            content: prompt,
          },
        ],
      });
      finalPrompt = completion.choices[0]?.message?.content ?? prompt;
    }

    const songId = await generateSong({
      prompt: finalPrompt,
      tags: style ?? "",
      title: title ?? "",
    });

    return NextResponse.json({ songId, status: "submitted" });
  } catch (err) {
    const message = err instanceof Error ? err.message : "Unknown error";
    return NextResponse.json({ error: message }, { status: 500 });
  }
}
