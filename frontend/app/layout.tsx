import type { Metadata } from "next";
import { Noto_Sans_Arabic } from "next/font/google";
import "./globals.css";
import { Header } from "@/components/layout/header";

const notoSansArabic = Noto_Sans_Arabic({
  subsets: ["arabic"],
  variable: "--font-arabic",
  display: "swap",
});

export const metadata: Metadata = {
  title: "المحرك الصرفي | Moteur Morphologique Arabe",
  description: "محرك بحث صرفي للغة العربية مع خوارزميات AVL و Hash Table - توليد وتحقق من الكلمات المشتقة",
  keywords: ["العربية", "صرف", "جذور", "أوزان", "محرك بحث", "AVL", "Hash Table"],
};

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html lang="ar" dir="rtl" className={notoSansArabic.variable}>
      <body className="min-h-screen bg-gradient-to-br from-gray-50 to-teal-50 font-arabic">
        <Header />
        <main className="container mx-auto px-4 py-8">
          {children}
        </main>
        <footer className="text-center py-6 text-gray-500 text-sm font-arabic">
          <p>مشروع تخرج - هندسة البرمجيات ونظم المعلومات 2025-2026</p>
        </footer>
      </body>
    </html>
  );
}
