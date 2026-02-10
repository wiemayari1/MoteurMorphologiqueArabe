import { Component } from '@angular/core';
import { MatCardModule } from '@angular/material/card';
import { MatButtonModule } from '@angular/material/button';

type Question = { root: string; scheme: string; answer: string };

@Component({
  standalone: true,
  selector: 'app-game-page',
  imports: [MatCardModule, MatButtonModule],
  templateUrl: './game-page.component.html',
  styleUrls: ['./game-page.component.scss']
})
export class GamePageComponent {
  questions: Question[] = [
    { root: 'كتب', scheme: 'فاعل', answer: 'كاتب' },
    { root: 'كتب', scheme: 'مفعول', answer: 'مكتوب' }
  ];

  i = 0;
  score = 0;
  finished = false;

  pick(choice: 'A' | 'B') {
    // mini logique (à remplacer par appel API si tu veux génération dynamique)
    const q = this.questions[this.i];
    const correct = 'A';
    if (choice === correct) this.score += 1;

    this.i += 1;
    if (this.i >= this.questions.length) this.finished = true;
  }

  reset() {
    this.i = 0;
    this.score = 0;
    this.finished = false;
  }
}
