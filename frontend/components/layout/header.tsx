"use client";

import Link from "next/link";
import { usePathname } from "next/navigation";
import { cn } from "@/lib/utils";
import { BookOpen, Menu, X } from "lucide-react";
import { useState } from "react";
import { Button } from "@/components/ui/button";

const navItems = [
  { href: "/", label: "الرئيسية", icon: "🏠" },
  { href: "/validate", label: "التحقق", icon: "🔍" },
  { href: "/roots", label: "الجذور", icon: "🌿" },
  { href: "/schemes", label: "الأوزان", icon: "⚖️" },
  { href: "/generate", label: "التوليد", icon: "✨" },
  { href: "/game", label: "اللعبة", icon: "🎮" },
];

export function Header() {
  const pathname = usePathname();
  const [mobileMenuOpen, setMobileMenuOpen] = useState(false);

  return (
    <header className="sticky top-0 z-50 w-full bg-teal-800 shadow-lg">
      <div className="container mx-auto px-4 h-16 flex items-center justify-between">
        {/* Logo */}
        <Link href="/" className="flex items-center gap-2 text-white">
          <BookOpen className="h-8 w-8" />
          <span className="text-xl font-bold font-arabic">المحرك الصرفي</span>
        </Link>

        {/* Desktop Navigation */}
        <nav className="hidden md:flex items-center gap-1">
          {navItems.map((item) => (
            <Link
              key={item.href}
              href={item.href}
              className={cn(
                "px-4 py-2 rounded-lg text-sm font-medium transition-colors font-arabic",
                pathname === item.href
                  ? "bg-teal-700 text-white"
                  : "text-teal-100 hover:bg-teal-700/50 hover:text-white"
              )}
            >
              {item.label}
            </Link>
          ))}
        </nav>

        {/* Mobile Menu Button */}
        <Button
          variant="ghost"
          size="icon"
          className="md:hidden text-white hover:bg-teal-700"
          onClick={() => setMobileMenuOpen(!mobileMenuOpen)}
        >
          {mobileMenuOpen ? <X className="h-6 w-6" /> : <Menu className="h-6 w-6" />}
        </Button>
      </div>

      {/* Mobile Navigation */}
      {mobileMenuOpen && (
        <div className="md:hidden bg-teal-800 border-t border-teal-700">
          <nav className="container mx-auto px-4 py-4 flex flex-col gap-2">
            {navItems.map((item) => (
              <Link
                key={item.href}
                href={item.href}
                onClick={() => setMobileMenuOpen(false)}
                className={cn(
                  "px-4 py-3 rounded-lg text-right font-arabic transition-colors",
                  pathname === item.href
                    ? "bg-teal-700 text-white"
                    : "text-teal-100 hover:bg-teal-700/50"
                )}
              >
                <span className="ml-2">{item.icon}</span>
                {item.label}
              </Link>
            ))}
          </nav>
        </div>
      )}
    </header>
  );
}
