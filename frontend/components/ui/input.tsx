import * as React from "react";
import { cn } from "@/lib/utils";
import { LucideIcon } from "lucide-react";

export interface InputProps extends React.InputHTMLAttributes<HTMLInputElement> {
  icon?: LucideIcon;
  error?: string;
  label?: string;
}

const Input = React.forwardRef<HTMLInputElement, InputProps>(
  ({ className, type, icon: Icon, error, label, ...props }, ref) => {
    return (
      <div className="relative w-full group">
        {label && (
          <label className="block text-sm font-medium text-gray-700 mb-2 text-right">
            {label}
          </label>
        )}
        <div className="relative">
          {Icon && (
            <div className="absolute left-3 top-1/2 -translate-y-1/2 text-gray-400 group-focus-within:text-teal-600 transition-colors">
              <Icon size={18} />
            </div>
          )}
          <input
            type={type}
            className={cn(
              "flex w-full rounded-xl border bg-white/80 backdrop-blur-sm px-4 py-3 text-base shadow-sm transition-all duration-200",
              "placeholder:text-gray-400",
              "focus:outline-none focus:ring-2 focus:ring-teal-500/20 focus:border-teal-500",
              "disabled:cursor-not-allowed disabled:opacity-50",
              Icon ? "pl-10" : "",
              error ? "border-red-500 focus:border-red-500 focus:ring-red-500/20" : "border-gray-200",
              "text-right font-arabic",
              className
            )}
            ref={ref}
            dir="rtl"
            {...props}
          />
        </div>
        {error && (
          <p className="mt-1.5 text-sm text-red-600 text-right">{error}</p>
        )}
      </div>
    );
  }
);
Input.displayName = "Input";

export { Input };
