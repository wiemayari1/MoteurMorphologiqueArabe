import { Loader2 } from "lucide-react";
import { cn } from "@/lib/utils";

interface SpinnerProps {
  size?: "sm" | "md" | "lg";
  text?: string;
  className?: string;
}

export function Spinner({ size = "md", text, className }: SpinnerProps) {
  const sizeClasses = {
    sm: "w-4 h-4",
    md: "w-8 h-8",
    lg: "w-12 h-12",
  };

  return (
    <div className={cn("flex flex-col items-center justify-center gap-3", className)}>
      <Loader2 className={cn("animate-spin text-teal-600", sizeClasses[size])} />
      {text && (
        <p className="text-gray-600 font-arabic text-sm animate-pulse">{text}</p>
      )}
    </div>
  );
}
