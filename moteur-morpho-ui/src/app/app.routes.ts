import { Routes } from '@angular/router';
import { HomePageComponent } from './pages/home/home-page.component';
import { RootsPageComponent } from './pages/roots/roots-page.component';
import { SchemesPageComponent } from './pages/schemes/schemes-page.component';
import { GeneratePageComponent } from './pages/generate/generate-page.component';
import { ValidatePageComponent } from './pages/validate/validate-page.component';
import { GamePageComponent } from './pages/game/game-page.component';

export const routes: Routes = [
  { path: '', component: HomePageComponent },
  { path: 'roots', component: RootsPageComponent },
  { path: 'schemes', component: SchemesPageComponent },
  { path: 'generate', component: GeneratePageComponent },
  { path: 'validate', component: ValidatePageComponent },
  { path: 'game', component: GamePageComponent },
  { path: 'settings', loadComponent: () => import('./pages/settings/settings-page.component').then(m => m.SettingsPageComponent) },
  { path: '**', redirectTo: '' }
];
