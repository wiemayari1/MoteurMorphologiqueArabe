// frontend/app/schemes/layout.tsx 
export const metadata = {
  title: 'إدارة الأوزان',
  description: 'أضف وعدل الأوزان الصرفية مع قواعد التحويل',
}

export default function SchemesLayout({
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
