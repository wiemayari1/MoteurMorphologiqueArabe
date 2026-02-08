import { Component, inject } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { MorphoService } from './services/morpho';

interface Derive {
  mot: string;
  freq: number;
}

@Component({
  selector: 'app-root',
  standalone: true,
  imports: [CommonModule, FormsModule],
  templateUrl: './app.html',
  styleUrl: './app.css'
})
export class AppComponent {

  private morphoService = inject(MorphoService);

  // --- 1. Gestion des racines ---
  messageRacines = '';
  racineGen = '';

  // --- 2. Gestion des schèmes (comme une table de hachage simplifiée) ---
  schemes: string[] = ['مفعول', 'فاعل', 'تفعيل', 'افتعل'];
  nouveauScheme = '';

  // --- 3. Génération ---
  schemeGen = '';
  resultatGen = '';
  erreurGen = '';

  // --- 4. Validation ---
  motVal = '';
  racineVal = '';
  resultatVal = '';
  detailVal = '';

  // --- 5. Liste des dérivés validés (par racine) ---
  racineListe = '';
  derives: Derive[] = [];

  // --- 6. Jeu ---
  question = 'مكتوب';           // mot dérivé affiché
  bonneRacine = 'كتب';
  bonScheme = 'مفعول';

  reponseRacine = '';
  reponseScheme = '';
  message = '';
  score = 0;
  total = 1;

  constructor() {
    this.nouvelleQuestion();
  }

  // 1) Ajout d'une racine (simule insertion dans arbre)
  ajouterRacine() {
    if (!this.racineGen.trim()) {
      this.messageRacines = 'الرجاء إدخال جذر ثلاثي.';
      return;
    }
    // Plus tard : appel à ton moteur C++ (AVL) via API ou exécutable.
    this.messageRacines = 'تمّ إدخال الجذر «' + this.racineGen + '» في الشجرة.';
  }

  // 2) Gestion des schèmes (comme si c'était une table de hachage)
  ajouterScheme() {
    const s = this.nouveauScheme.trim();
    if (!s) {
      return;
    }
    if (!this.schemes.includes(s)) {
      this.schemes.push(s);
    }
    this.nouveauScheme = '';
  }

  // 3) Génération
  generer() {
    this.erreurGen = '';
    this.resultatGen = '';

    if (!this.racineGen || !this.schemeGen) {
      this.erreurGen = 'الرجاء إدخال الجذر والوزن.';
      return;
    }

    this.morphoService.genererMot({ racine: this.racineGen, scheme: this.schemeGen })
      .subscribe(res => {
        this.resultatGen = res.mot;

        // Exemple : mise à jour de la liste des dérivés pour الجذر كتب
        if (this.racineGen === 'كتب') {
          this.derives = [
            { mot: 'مكتب', freq: 5 },
            { mot: 'كاتب', freq: 3 },
            { mot: 'مكتوب', freq: 7 }
          ];
        }
      });
  }

  // 4) Validation morphologique
  verifier() {
    this.resultatVal = '';
    this.detailVal = '';

    if (!this.motVal || !this.racineVal) {
      this.resultatVal = 'الرجاء إدخال الكلمة والجذر.';
      return;
    }

    this.morphoService.validerMot(this.motVal, this.racineVal)
      .subscribe(res => {
        if (res.appartient) {
          this.resultatVal = 'نعم، الكلمة تنتمي إلى هذا الجذر.';
          if (res.scheme) {
            this.detailVal = 'الوزن المستعمل: ' + res.scheme;
          }
        } else {
          this.resultatVal = 'لا، الكلمة لا تنتمي إلى هذا الجذر.';
        }
      });
  }

  // 5) Affichage des dérivés pour un gène
  afficherDerives() {
    if (this.racineListe.trim() === 'كتب') {
      this.derives = [
        { mot: 'مكتب', freq: 5 },
        { mot: 'كاتب', freq: 3 },
        { mot: 'مكتوب', freq: 7 }
      ];
    } else {
      this.derives = [];
    }
  }

  // 6) Jeu : nouvelle question avec quelques mots simples
  nouvelleQuestion() {
    const questions = [
      { mot: 'مكتوب', racine: 'كتب', scheme: 'مفعول' },
      { mot: 'كاتب', racine: 'كتب', scheme: 'فاعل' },
      { mot: 'مدرسة', racine: 'درس', scheme: 'مفعلة' }
    ];
    const q = questions[Math.floor(Math.random() * questions.length)];
    this.question = q.mot;
    this.bonneRacine = q.racine;
    this.bonScheme = q.scheme;
    this.reponseRacine = '';
    this.reponseScheme = '';
    this.message = '';
    this.total++;
  }

  validerJeu() {
    const rOK = this.reponseRacine.trim() === this.bonneRacine;
    const sOK = this.reponseScheme.trim() === this.bonScheme;

    if (rOK && sOK) {
      this.score++;
      this.message = 'إجابة صحيحة! الجذر «' + this.bonneRacine +
        '» على الوزن «' + this.bonScheme + '».';
    } else {
      this.message = 'إجابة غير ص��يحة. الجذر الصحيح «' +
        this.bonneRacine + '» والوزن «' + this.bonScheme + '».';
    }

    this.nouvelleQuestion();
  }
}
