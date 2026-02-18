"use client";

import Link from "next/link";
import { Card, CardContent } from "@/components/ui/card";
import { Badge } from "@/components/ui/badge";
import { Button } from "@/components/ui/button";
import { 
  Gamepad2, 
  Search, 
  Sparkles, 
  Grid3X3, 
  TreePine,
  Zap,
  Quote
} from "lucide-react";

const features = [
  {
    href: "/game",
    icon: Gamepad2,
    label: "اللعبة التعليمية",
    description: "اختبر معرفتك الصرفية ",
    color: "bg-rose-100 text-rose-600",
    hoverColor: "group-hover:bg-rose-200",
    badge: "جديد",
  },
  {
    href: "/validate",
    icon: Search,
    label: "التحقق من الكلمات",
    description: "تحقق من انتماء كلمة لجذرها الصرفي",
    color: "bg-blue-100 text-blue-600",
    hoverColor: "group-hover:bg-blue-200",
  },
  {
    href: "/generate",
    icon: Sparkles,
    label: "توليد العائلة الصرفية",
    description: "ولد جميع الكلمات المشتقة من جذر معين",
    color: "bg-amber-100 text-amber-600",
    hoverColor: "group-hover:bg-amber-200",
  },
  {
    href: "/schemes",
    icon: Grid3X3,
    label: "إدارة الأوزان",
    description: "أضف وعدل الأوزان الصرفية مع قواعد التحويل",
    color: "bg-purple-100 text-purple-600",
    hoverColor: "group-hover:bg-purple-200",
  },
  {
    href: "/roots",
    icon: TreePine,
    label: "إدارة الجذور",
    description: "أضف وحذف الجذور العربية (شجرة AVL)",
    color: "bg-emerald-100 text-emerald-600",
    hoverColor: "group-hover:bg-emerald-200",
  },
];

export default function HomePage() {
  return (
    <div className="space-y-16">
      {/* Features Grid */}
      <div>
        <div className="grid grid-cols-2 md:grid-cols-3 lg:grid-cols-5 gap-4 md:gap-6">
          {features.map((feature) => (
            <Link key={feature.href} href={feature.href} className="group">
              <Card className="h-full transition-all duration-300 hover:shadow-glass-lg hover:-translate-y-2 border-0 relative overflow-hidden">
                {feature.badge && (
                  <Badge className="absolute top-2 left-2 bg-rose-500 text-white text-xs">
                    {feature.badge}
                  </Badge>
                )}
                <CardContent className="flex flex-col items-center justify-center p-6 text-center h-full">
                  <div
                    className={`
                      w-16 h-16 rounded-2xl flex items-center justify-center mb-4
                      transition-colors duration-300 ${feature.color} ${feature.hoverColor}
                    `}
                  >
                    <feature.icon className="w-8 h-8" />
                  </div>
                  <h3 className="text-lg font-bold text-gray-800 mb-2 font-arabic group-hover:text-teal-700 transition-colors">
                    {feature.label}
                  </h3>
                  <p className="text-sm text-gray-500 font-arabic leading-relaxed">
                    {feature.description}
                  </p>
                </CardContent>
              </Card>
            </Link>
          ))}
        </div>
      </div>

      {/* Poem Section */}
      <div className="relative">
        <div className="absolute inset-0 bg-gradient-to-r from-teal-600/5 to-purple-600/5 rounded-3xl" />
        <div className="relative glass rounded-2xl p-8 md:p-12 text-center">
          <div className="flex justify-center mb-6">
            <div className="w-16 h-16 rounded-full bg-teal-100 flex items-center justify-center">
              <Quote className="w-8 h-8 text-teal-600" />
            </div>
          </div>

          <blockquote className="space-y-4 mb-6">
            <p className="text-xl md:text-2xl font-arabic text-teal-900 leading-loose">
              حَقُّ العَشيرَةِ في نُبوغِكَ أَوَّلٌ
              <br />
              فَاِنظُر لَعَلَّكَ بِالعَشيرَةِ بادي
            </p>
            <p className="text-xl md:text-2xl font-arabic text-teal-900 leading-loose">
              لَم يَكفِهِم شَطرُ النُبوغِ فَزُدهُمُ
              <br />
              إِن كُنتَ بِالشَطرَينِ غَيرَ جَوادِ
            </p>
            <p className="text-xl md:text-2xl font-arabic text-teal-900 leading-loose">
              أَو دَع لِسانَكَ وَاللُغاتِ فَرُبَّما
              <br />
              غَنّى الأَصيلُ بِمَنطِقِ الأَجدادِ
            </p>
            <p className="text-xl md:text-2xl font-arabic text-teal-900 leading-loose">
              إِنَّ الَّذي مَلَأَ اللُغاتِ مَحاسِناً
              <br />
              جَعَلَ الجَمالَ وَسَرَّهُ في الضادِ
            </p>
          </blockquote>

          <div className="flex items-center justify-center gap-2 text-gray-600">
            <div className="h-px w-12 bg-gray-300" />
            <span className="font-arabic text-lg">أحمد شوقي</span>
            <div className="h-px w-12 bg-gray-300" />
          </div>
        </div>
      </div>
    </div>
  );
}
