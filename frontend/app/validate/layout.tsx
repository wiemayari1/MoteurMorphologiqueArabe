export const metadata = {
  title: 'التحقق من الكلمات',
  description: 'تحقق من انتماء كلمة لجذرها الصرفي',
}

export default function ValidateLayout({
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
