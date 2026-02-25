export const metadata = {
  title: 'توليد الكلمات',
  description: 'ولد جميع الكلمات المشتقة من جذر معين',
}

export default function GenerateLayout({
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
