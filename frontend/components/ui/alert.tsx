import * as React from "react";
import { cva, type VariantProps } from "class-variance-authority";
import { AlertCircle, CheckCircle, Info } from "lucide-react";
import { cn } from "@/lib/utils";

const alertVariants = cva(
  "relative w-full rounded-xl border p-4 [&>svg]:absolute [&>svg]:right-4 [&>svg]:top-4 [&>svg]:h-5 [&>svg]:w-5 [&>svg+div]:translate-x-0",
  {
    variants: {
      variant: {
        default: "bg-gray-50 text-gray-900 border-gray-200",
        success: "bg-green-50 text-green-900 border-green-200 [&>svg]:text-green-600",
        error: "bg-red-50 text-red-900 border-red-200 [&>svg]:text-red-600",
        warning: "bg-amber-50 text-amber-900 border-amber-200 [&>svg]:text-amber-600",
        info: "bg-teal-50 text-teal-900 border-teal-200 [&>svg]:text-teal-600",
      },
    },
    defaultVariants: {
      variant: "default",
    },
  }
);

const icons = {
  default: Info,
  success: CheckCircle,
  error: AlertCircle,
  warning: AlertCircle,
  info: Info,
};

interface AlertProps extends React.HTMLAttributes<HTMLDivElement>, VariantProps<typeof alertVariants> {
  title?: string;
}

const Alert = React.forwardRef<HTMLDivElement, AlertProps>(
  ({ className, variant = "default", title, children, ...props }, ref) => {
    const Icon = icons[variant || "default"];
    
    return (
      <div
        ref={ref}
        role="alert"
        className={cn(alertVariants({ variant }), "text-right pr-12", className)}
        dir="rtl"
        {...props}
      >
        <Icon className="h-5 w-5" />
        <div className="space-y-1">
          {title && <h5 className="font-medium leading-none tracking-tight">{title}</h5>}
          <div className="text-sm opacity-90">{children}</div>
        </div>
      </div>
    );
  }
);
Alert.displayName = "Alert";

export { Alert, alertVariants };
