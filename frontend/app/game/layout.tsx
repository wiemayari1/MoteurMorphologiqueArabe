// frontend/app/game/layout.tsx - VERSION CORRIGÉE
export const metadata = {
  title: 'اللعبة التعليمية',
  description: 'اختبر معرفتك الصرفية',
}

export default function GameLayout({
  children,
}: {
  children: React.ReactNode
}) {
  return (
    <div className="min-h-screen bg-gradient-to-br from-gray-50 to-teal-50">
      {children}
    </div>
  )
}
