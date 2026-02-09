import { Routes } from '@angular/router';
import { HomeComponent } from './pages/home/home.component';
import { GenerateComponent } from './pages/generate/generate.component';
import { ValidateComponent } from './pages/validate/validate.component';
import { GameComponent } from './pages/game/game.component';

export const routes: Routes = [
  { path: '', component: HomeComponent },
  { path: 'generate', component: GenerateComponent },
  { path: 'validate', component: ValidateComponent },
  { path: 'game', component: GameComponent },
  { path: '**', redirectTo: '' },
];
