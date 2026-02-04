#include <QApplication>
#include <QMainWindow>
#include <QTabWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QMessageBox>

#include <fstream>
#include <sstream>
#include <string>

#include "morpho.h"
#include "AVL.h"
#include "hash_table.h"

// Fonction utilitaire pour lire un fichier UTF-8 complet
static std::string read_file_utf8(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        return "";
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// Fenêtre principale
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(AVLTree& tree, HashTable& ht, QWidget* parent = nullptr)
        : QMainWindow(parent), tree(tree), ht(ht)
    {
        auto* tabs = new QTabWidget(this);

        tabs->addTab(createGenerationTab(), tr("Génération"));
        tabs->addTab(createValidationTab(), tr("Validation"));
        // Plus tard : createRootsTab(), createGameTab() ...

        setCentralWidget(tabs);
        setWindowTitle("Moteur Morphologique Arabe - Interface Graphique");
        resize(800, 600);
    }

private:
    AVLTree& tree;
    HashTable& ht;

    QWidget* createGenerationTab() {
        auto* w = new QWidget;
        auto* layout = new QVBoxLayout(w);
        auto* form = new QFormLayout;

        auto* rootEdit = new QLineEdit;
        rootEdit->setPlaceholderText("ex: كتب");
        auto* schemeBox = new QComboBox;
        auto* resultLabel = new QLabel("Mot généré : ");

        // Remplir la combo avec les schèmes de la table de hachage
        for (const auto& s : ht.allSchemes()) {
            schemeBox->addItem(QString::fromStdString(u32_to_utf8(s.name)));
        }

        form->addRow("Racine :", rootEdit);
        form->addRow("Schème :", schemeBox);

        auto* genButton = new QPushButton("Générer");
        layout->addLayout(form);
        layout->addWidget(genButton);
        layout->addWidget(resultLabel);
        layout->addStretch();

        QObject::connect(genButton, &QPushButton::clicked, this,
                         [=, this]() {
            std::string rootStr = rootEdit->text().toStdString();
            std::string schemeNameStr = schemeBox->currentText().toStdString();

            if (rootStr.empty() || schemeNameStr.empty()) {
                QMessageBox::warning(this, "Attention",
                                     "Veuillez saisir une racine et choisir un schème.");
                return;
            }

            auto r_u32 = normalize_ar(utf8_to_u32(rootStr));
            auto sname_u32 = utf8_to_u32(schemeNameStr);

            SchemeEntry* se = ht.get(sname_u32);
            if (!se) {
                QMessageBox::critical(this, "Erreur", "Schème introuvable dans la table.");
                return;
            }

            try {
                auto word = apply_template(r_u32, se->templ);
                resultLabel->setText(
                    QString::fromStdString("Mot généré : " + u32_to_utf8(word))
                );
            } catch (const std::exception& e) {
                QMessageBox::critical(this, "Erreur",
                                      QString("Erreur lors de la génération : ") + e.what());
            }
        });

        return w;
    }

    QWidget* createValidationTab() {
        auto* w = new QWidget;
        auto* layout = new QVBoxLayout(w);
        auto* form = new QFormLayout;

        auto* wordEdit = new QLineEdit;
        wordEdit->setPlaceholderText("Mot (ex: مكتوب)");
        auto* rootEdit = new QLineEdit;
        rootEdit->setPlaceholderText("Racine candidate (ex: كتب)");
        auto* resultLabel = new QLabel("Résultat : ");

        form->addRow("Mot :", wordEdit);
        form->addRow("Racine :", rootEdit);

        auto* checkButton = new QPushButton("Vérifier");
        layout->addLayout(form);
        layout->addWidget(checkButton);
        layout->addWidget(resultLabel);
        layout->addStretch();

        QObject::connect(checkButton, &QPushButton::clicked, this,
                         [=, this]() {
            std::string w_str = wordEdit->text().toStdString();
            std::string r_str = rootEdit->text().toStdString();

            if (w_str.empty() || r_str.empty()) {
                QMessageBox::warning(this, "Attention",
                                     "Veuillez saisir un mot et une racine.");
                return;
            }

            auto w_u32 = utf8_to_u32(w_str);
            auto r_u32 = normalize_ar(utf8_to_u32(r_str));

            bool appartient = false;
            std::string schemes_str;

            for (const auto& s : ht.allSchemes()) {
                auto maybe_r = extract_root_from_word(w_u32, s.templ);
                if (maybe_r) {
                    auto rn = normalize_ar(*maybe_r);
                    if (rn == r_u32) {
                        appartient = true;
                        schemes_str += u32_to_utf8(s.name) + " ";
                    }
                }
            }

            if (!appartient) {
                resultLabel->setText("Résultat : NON, le mot ne correspond pas à cette racine.");
            } else {
                resultLabel->setText(
                    QString::fromStdString("Résultat : OUI. Schème(s) : " + schemes_str)
                );
            }
        });

        return w;
    }
};

#include "gui_main.moc"

// --- main GUI (chargement des données + lancement Qt) ---

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    if (argc < 3) {
        QMessageBox::critical(nullptr, "Erreur",
                              "Usage: gui_morpho data/roots.txt data/schemes.txt");
        return 1;
    }

    std::string roots_path   = argv[1];
    std::string schemes_path = argv[2];

    AVLTree tree;
    HashTable ht(2048);

    // Chargement des racines
    {
        std::string content = read_file_utf8(roots_path);
        std::istringstream in(content);
        std::string line;
        while (std::getline(in, line)) {
            if (line.empty()) continue;
            auto r_u32 = normalize_ar(utf8_to_u32(line));
            if (r_u32.size() == 3) {
                tree.insert(r_u32);
            }
        }
    }

    // Chargement des schèmes (NOM|TEMPLATE)
    {
        std::string content = read_file_utf8(schemes_path);
        std::istringstream in(content);
        std::string line;
        while (std::getline(in, line)) {
            if (line.empty()) continue;
            auto pos = line.find('|');
            if (pos == std::string::npos) continue;
            std::string name  = line.substr(0, pos);
            std::string templ = line.substr(pos + 1);

            auto name_u32  = utf8_to_u32(name);
            auto templ_u32 = utf8_to_u32(templ);
            ht.put(name_u32, templ_u32);
        }
    }

    MainWindow win(tree, ht);
    win.show();

    return app.exec();
}