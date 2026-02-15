"use client";

import { useEffect, useState } from "react";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Button } from "@/components/ui/button";
import { Badge } from "@/components/ui/badge";
import { Alert } from "@/components/ui/alert";
import { Spinner } from "@/components/ui/spinner";
import { getGameQuestions, submitAnswer } from "@/lib/api";
import type { GameQuestion } from "@/lib/types";
import { Gamepad2, Trophy, RotateCcw, CheckCircle, XCircle } from "lucide-react";

interface AnswerRecord {
  questionId: number;
  selected: string;
  correct: boolean;
  correctAnswer: string;
}

export default function GamePage() {
  const [questions, setQuestions] = useState<GameQuestion[]>([]);
  const [currentIndex, setCurrentIndex] = useState(0);
  const [loading, setLoading] = useState(true);
  const [selectedAnswer, setSelectedAnswer] = useState<string | null>(null);
  const [checking, setChecking] = useState(false);
  const [answers, setAnswers] = useState<AnswerRecord[]>([]);
  const [gameFinished, setGameFinished] = useState(false);
  const [error, setError] = useState("");

  useEffect(() => {
    startNewGame();
  }, []);

  const startNewGame = async () => {
    setLoading(true);
    setError("");
    setCurrentIndex(0);
    setAnswers([]);
    setGameFinished(false);
    setSelectedAnswer(null);
    
    const response = await getGameQuestions();
    console.log("Game response:", response);
    
    if (response.success && response.data && response.data.questions) {
      setQuestions(response.data.questions);
    } else {
      setError(response.error || "فشل في تحميل الأسئلة. تأكد من وجود جذور وأوزان في النظام.");
    }
    setLoading(false);
  };

  const currentQuestion = questions[currentIndex];

  const handleAnswer = async (answer: string) => {
    if (selectedAnswer || checking || !currentQuestion) return;
    
    setSelectedAnswer(answer);
    setChecking(true);

    console.log("Submitting answer:", currentQuestion.id, answer);
    const response = await submitAnswer(currentQuestion.id, answer);
    console.log("Answer response:", response);
    
    const isCorrect = response.data?.correct || false;
    const correctAnswer = response.data?.correctAnswer || "";

    setAnswers(prev => [...prev, {
      questionId: currentQuestion.id,
      selected: answer,
      correct: isCorrect,
      correctAnswer: correctAnswer
    }]);

    setChecking(false);
  };

  const handleNext = () => {
    if (currentIndex < questions.length - 1) {
      setCurrentIndex(prev => prev + 1);
      setSelectedAnswer(null);
    } else {
      setGameFinished(true);
    }
  };

  const getQuestionTypeText = (type: string) => {
    switch (type) {
      case "find_root": return "ما هو جذر هذه الكلمة؟";
      case "find_scheme": return "ما هو الوزن الصرفي لهذه الكلمة؟";
      case "validate_word": return "هل هذه الكلمة تنتمي لهذا الجذر؟";
      default: return "";
    }
  };

  const getQuestionContext = (q: GameQuestion) => {
    switch (q.type) {
      case "find_root":
        return { main: q.word, sub: `الوزن: ${q.scheme_name}` };
      case "find_scheme":
        return { main: q.word, sub: `الجذر: ${q.root}` };
      case "validate_word":
        return { main: q.word, sub: `الجذر المقترح: ${q.root}` };
      default:
        return { main: "", sub: "" };
    }
  };

  const score = answers.filter(a => a.correct).length;
  const totalAnswered = answers.length;

  if (loading) {
    return (
      <div className="flex justify-center py-20">
        <Spinner size="lg" text="جاري تحميل الأسئلة..." />
      </div>
    );
  }

  if (error) {
    return (
      <div className="max-w-2xl mx-auto">
        <Alert variant="error">{error}</Alert>
        <Button onClick={startNewGame} className="mt-4 w-full">
          <RotateCcw className="w-4 h-4 mr-2" />
          إعادة المحاولة
        </Button>
      </div>
    );
  }

  if (gameFinished) {
    const percentage = questions.length > 0 ? Math.round((score / questions.length) * 100) : 0;
    let message = "";
    let color = "";
    
    if (percentage >= 80) {
      message = "🎉 ممتاز! أحسنت";
      color = "text-green-600";
    } else if (percentage >= 60) {
      message = "👍 جيد جداً";
      color = "text-blue-600";
    } else if (percentage >= 40) {
      message = "💪 يمكنك التحسن";
      color = "text-amber-600";
    } else {
      message = "📚 حاول مرة أخرى";
      color = "text-red-600";
    }

    return (
      <div className="max-w-2xl mx-auto animate-slide-up">
        <Card className="border-0 shadow-glass">
          <CardHeader className="text-center">
            <CardTitle className="text-3xl font-arabic text-teal-900">
              نتيجة اللعبة
            </CardTitle>
          </CardHeader>
          <CardContent className="space-y-6 text-center">
            <div className="py-8">
              <Trophy className={`w-20 h-20 mx-auto mb-4 ${color}`} />
              <p className={`text-4xl font-bold mb-2 ${color}`}>{percentage}%</p>
              <p className="text-xl font-arabic text-gray-700">{message}</p>
              <p className="text-gray-500 mt-2">
                {score} إجابة صحيحة من أصل {questions.length}
              </p>
            </div>

            <div className="text-right space-y-2">
              <h3 className="font-bold text-gray-700 mb-4">مراجعة الإجابات:</h3>
              {answers.map((ans, idx) => (
                <div 
                  key={idx} 
                  className={`p-3 rounded-lg flex items-center justify-between ${
                    ans.correct ? "bg-green-50" : "bg-red-50"
                  }`}
                >
                  <span className="text-gray-500">سؤال {idx + 1}</span>
                  <div className="flex items-center gap-2">
                    <span className="font-arabic">{ans.selected}</span>
                    {ans.correct ? (
                      <CheckCircle className="w-5 h-5 text-green-600" />
                    ) : (
                      <XCircle className="w-5 h-5 text-red-600" />
                    )}
                  </div>
                </div>
              ))}
            </div>

            <Button onClick={startNewGame} size="lg" className="w-full text-lg">
              <RotateCcw className="w-5 h-5 mr-2" />
              لعبة جديدة
            </Button>
          </CardContent>
        </Card>
      </div>
    );
  }

  if (!currentQuestion) {
    return <Alert variant="error">لا توجد أسئلة متاحة</Alert>;
  }

  const context = getQuestionContext(currentQuestion);
  const currentAnswer = answers.find(a => a.questionId === currentQuestion.id);

  return (
    <div className="max-w-2xl mx-auto animate-slide-up">
      <div className="mb-6">
        <div className="flex justify-between items-center mb-2">
          <span className="text-sm text-gray-500">
            السؤال {currentIndex + 1} من {questions.length}
          </span>
          <div className="flex items-center gap-2 bg-amber-100 px-4 py-2 rounded-full">
            <Trophy className="w-5 h-5 text-amber-600" />
            <span className="font-arabic font-bold text-amber-800">
              {score} / {totalAnswered}
            </span>
          </div>
        </div>
        <div className="w-full bg-gray-200 rounded-full h-2">
          <div 
            className="bg-teal-600 h-2 rounded-full transition-all"
            style={{ width: `${((currentIndex) / questions.length) * 100}%` }}
          />
        </div>
      </div>

      <Card className="border-0 shadow-glass">
        <CardHeader className="text-center">
          <Badge variant="outline" className="mb-2 font-arabic">
            {currentQuestion.difficulty === 'easy' ? 'سهل' : 
             currentQuestion.difficulty === 'medium' ? 'متوسط' : 'صعب'}
          </Badge>
          <CardTitle className="text-xl font-arabic text-teal-900">
            {getQuestionTypeText(currentQuestion.type)}
          </CardTitle>
        </CardHeader>
        
        <CardContent className="space-y-6">
          <div className="text-center py-6 bg-teal-50 rounded-lg">
            <h2 className="text-4xl font-bold text-teal-900 font-arabic mb-2">
              {context.main}
            </h2>
            <p className="text-gray-600 font-arabic">{context.sub}</p>
          </div>

          <div className={`grid gap-3 ${currentQuestion.type === 'validate_word' ? 'grid-cols-2' : 'grid-cols-2'}`}>
            {currentQuestion.options.map((option, index) => {
              let buttonClass = "h-16 text-xl font-arabic transition-all ";
              
              if (!currentAnswer) {
                buttonClass += "bg-white hover:bg-teal-50 border-2 border-gray-200 hover:border-teal-300";
              } else if (option === currentAnswer.correctAnswer) {
                buttonClass += "bg-green-100 border-2 border-green-500 text-green-800";
              } else if (option === currentAnswer.selected) {
                buttonClass += "bg-red-100 border-2 border-red-500 text-red-800";
              } else {
                buttonClass += "bg-gray-100 border-2 border-gray-200 opacity-50";
              }

              return (
                <Button
                  key={index}
                  onClick={() => handleAnswer(option)}
                  disabled={!!currentAnswer || checking}
                  className={buttonClass}
                  variant="ghost"
                >
                  {option}
                </Button>
              );
            })}
          </div>

          {currentAnswer && (
            <div className={`p-4 rounded-lg ${
              currentAnswer.correct ? "bg-green-50 border border-green-200" : "bg-red-50 border border-red-200"
            }`}>
              <div className="flex items-center gap-2 mb-2">
                {currentAnswer.correct ? (
                  <>
                    <CheckCircle className="w-5 h-5 text-green-600" />
                    <span className="font-bold text-green-800 font-arabic">إجابة صحيحة!</span>
                  </>
                ) : (
                  <>
                    <XCircle className="w-5 h-5 text-red-600" />
                    <span className="font-bold text-red-800 font-arabic">إجابة خاطئة</span>
                  </>
                )}
              </div>
              {!currentAnswer.correct && (
                <p className="text-red-700 font-arabic">
                  الإجابة الصحيحة: <strong>{currentAnswer.correctAnswer}</strong>
                </p>
              )}
            </div>
          )}

          {currentAnswer && (
            <Button 
              onClick={handleNext} 
              className="w-full text-lg font-arabic"
              size="lg"
            >
              {currentIndex < questions.length - 1 ? "السؤال التالي →" : "عرض النتيجة"}
            </Button>
          )}
        </CardContent>
      </Card>
    </div>
  );
}
