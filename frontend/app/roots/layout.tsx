// frontend/app/roots/layout.tsx - VERSION CORRIGÉE
export const metadata = {
    title: 'إدارة الجذور',
    description: 'أضف وحذف الجذور العربية',
}

export default function RootsLayout({
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