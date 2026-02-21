export interface Root {
  id: number;
  value: string;
  letters: string[];
  frequency: number;
  derived_count: number;
}

export interface Scheme {
  id: number;
  name: string;
  pattern: string;
  rule: string;
  rule_pattern?: string;
}

export interface GeneratedWord {
  root: string;
  scheme_name: string;
  scheme_pattern: string;
  result: string;
}

export interface ValidationResult {
  valid: boolean | string;
  word: string;
  root: string;
  message: string;
  complexity: string;
  scheme?: {
    name: string;
    pattern: string;
  };
}

export interface GameQuestion {
  id: number;
  type: "find_root" | "find_scheme" | "validate_word";
  word: string;
  root: string;
  scheme_name: string;
  difficulty: "easy" | "medium" | "hard";
  options: string[];
  correctRoot?: string;
}

export interface GameState {
  questions: GameQuestion[];
  currentIndex: number;
  score: number;
  answers: { questionId: number; correct: boolean }[];
}
