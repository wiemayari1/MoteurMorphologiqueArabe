import * as React from "react";
import { Slot } from "@radix-ui/react-slot";
import { cva, type VariantProps } from "class-variance-authority";
import { cn } from "@/lib/utils";

const buttonVariants = cva(
  "inline-flex items-center justify-center gap-2 whitespace-nowrap rounded-xl text-sm font-medium transition-all duration-300 focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-teal-500 disabled:pointer-events-none disabled:opacity-50 active:scale-95",
  {
    variants: {
      variant: {
        default: "bg-teal-700 text-white hover:bg-teal-800 shadow-lg hover:shadow-xl",
        destructive: "bg-red-600 text-white hover:bg-red-700",
        outline: "border-2 border-teal-700 bg-transparent text-teal-700 hover:bg-teal-50",
        secondary: "bg-teal-100 text-teal-900 hover:bg-teal-200",
        ghost: "hover:bg-teal-100 text-teal-700",
        link: "text-teal-700 underline-offset-4 hover:underline",
        glass: "bg-glass-white/90 backdrop-blur-md border border-glass-border text-teal-800 shadow-glass hover:shadow-glass-lg hover:-translate-y-1",
      },
      size: {
        default: "h-11 px-6 py-2",
        sm: "h-9 px-4 text-xs",
        lg: "h-12 px-8 text-base",
        icon: "h-11 w-11",
        full: "h-12 w-full px-6",
      },
    },
    defaultVariants: {
      variant: "default",
      size: "default",
    },
  }
);

export interface ButtonProps
  extends React.ButtonHTMLAttributes<HTMLButtonElement>,
    VariantProps<typeof buttonVariants> {
  asChild?: boolean;
}

const Button = React.forwardRef<HTMLButtonElement, ButtonProps>(
  ({ className, variant, size, asChild = false, ...props }, ref) => {
    const Comp = asChild ? Slot : "button";
    return (
      <Comp
        className={cn(buttonVariants({ variant, size, className }))}
        ref={ref}
        {...props}
      />
    );
  }
);
Button.displayName = "Button";

export { Button, buttonVariants };
