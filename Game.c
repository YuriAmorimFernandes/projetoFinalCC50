
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>   // Para usleep (pausas)
#include <ncurses.h>  // Biblioteca de manipulação de terminal

// --- Constantes Globais ---
#define MAX_RANQUE 5
#define MAX_NOME 6 // 5 caracteres + '\0'
#define NUM_PALAVRAS 100
#define MAX_PALAVRAS_SIMULTANEAS 10
#define VIDAS_INICIAIS 5

// Definição de Tempo (em milissegundos simulados)
// 625ms (inicialmente 1 linha a cada 0.625s) -> 15s para cair 24 linhas
#define TEMPO_BASE_QUEDA_MS 625

// --- Estruturas de Dados ---
typedef struct {
    char nome[MAX_NOME];
    int pontuacao;
} Recorde;

typedef struct {
    char *palavra;     // Palavra atual a ser digitada
    int y_pos;         // Posição vertical (linha)
    int x_pos;         // Posição horizontal (coluna)
    int progresso;     // Quantos caracteres já foram digitados (0 a N)
    int ativa;         // 1: Caindo; 0: Inativa/Digitada
} PalavraCaindo;

// Ranques e Palavras Ativas
Recorde ranque_facil[MAX_RANQUE];
Recorde ranque_medio[MAX_RANQUE];
Recorde ranque_dificil[MAX_RANQUE];
PalavraCaindo palavras_ativas[MAX_PALAVRAS_SIMULTANEAS];

// --- Lista de 100 Palavras (Português Brasil) ---
const char *PALAVRAS[NUM_PALAVRAS] = {
    "amor", "fato", "viés", "você", "mito", "como", "caos", "esmo", "brio", "ação",
    "vida", "casa", "medo", "saga", "ônus", "auge", "sina", "vovó", "ermo", "mais",
    "além", "pela", "mote", "tolo", "urge", "coragem", "liberdade", "empatia", "ética", "virtude",
    "sucesso", "sentido", "origem", "história", "cultura", "código", "projeto", "sistema", "erro", "teste",
    "teoria", "prática", "ideia", "missão", "desafio", "solução", "ambiente", "recurso", "trabalho", "inovação",
    "harmonia", "confiança", "tolerância", "democracia", "igualdade", "paciência", "felicidade", "destino", "equidade", "transformar",
    "mudança", "sabedoria", "conhecer", "intenção", "otimismo", "realidade", "espírito", "caminho", "foco", "meta",
    "progresso", "esforço", "objetivo", "reflexão", "responsável", "respeito", "dúvida", "sintaxe", "algoritmo", "variável",
    "função", "estrutura", "ponteiro", "memória", "compilar", "terminal", "processo", "desenho", "interface", "conexão",
    "efêmero", "imprevisto", "genuíno", "sublime", "ancestral", "presença", "estigma", "astúcia", "audácia", "fraternidade"
};

// --- Protótipos das Funções ---
void inicializar_ranques(Recorde ranque[]);
void carregar_ranques();
void salvar_ranques();
void menu_principal();
void menu_jogar();
void menu_ranque();
void spawn_palavra(int index, int max_x);
void jogar_facil();
void atualizar_ranque(Recorde ranque[], int nova_pontuacao, const char *modo_nome);
void mostrar_ranque(Recorde ranque[], const char *modo);

// --- Função Principal ---
int main() {
    srand(time(NULL));

    // Configuração do NCURSES (desativada no menu para uso de I/O padrão)

    // Inicialização e Carregamento
    inicializar_ranques(ranque_facil);
    inicializar_ranques(ranque_medio);
    inicializar_ranques(ranque_dificil);
    carregar_ranques();

    menu_principal();

    salvar_ranques();

    return 0;
}

// ----------------------------------------------------------------------
// Implementação: Gestão de Ranque (Persistência e Ordenação)
// ----------------------------------------------------------------------

void inicializar_ranques(Recorde ranque[]) {
    for (int i = 0; i < MAX_RANQUE; i++) {
        strcpy(ranque[i].nome, "#####");
        ranque[i].pontuacao = 0;
    }
}

void carregar_ranques() {
    FILE *fp;

    fp = fopen("ranque_facil.bin", "rb");
    if (fp != NULL) {
        fread(ranque_facil, sizeof(Recorde), MAX_RANQUE, fp);
        fclose(fp);
    }

    fp = fopen("ranque_medio.bin", "rb");
    if (fp != NULL) {
        fread(ranque_medio, sizeof(Recorde), MAX_RANQUE, fp);
        fclose(fp);
    }

    fp = fopen("ranque_dificil.bin", "rb");
    if (fp != NULL) {
        fread(ranque_dificil, sizeof(Recorde), MAX_RANQUE, fp);
        fclose(fp);
    }
}

void salvar_ranques() {
    FILE *fp;

    fp = fopen("ranque_facil.bin", "wb");
    if (fp != NULL) {
        fwrite(ranque_facil, sizeof(Recorde), MAX_RANQUE, fp);
        fclose(fp);
    }

    fp = fopen("ranque_medio.bin", "wb");
    if (fp != NULL) {
        fwrite(ranque_medio, sizeof(Recorde), MAX_RANQUE, fp);
        fclose(fp);
    }

    fp = fopen("ranque_dificil.bin", "wb");
    if (fp != NULL) {
        fwrite(ranque_dificil, sizeof(Recorde), MAX_RANQUE, fp);
        fclose(fp);
    }
}

void atualizar_ranque(Recorde ranque[], int nova_pontuacao, const char *modo_nome) {
    // Verifica se é um Top 5
    if (nova_pontuacao <= ranque[MAX_RANQUE - 1].pontuacao) {
        printf("\nVocê não entrou no Top 5 do Modo %s.\n", modo_nome);
        return;
    }

    // Coleta o nome
    char nome_temp[MAX_NOME];

    printf("\n?? NOVO RECORDE! Você está no TOP 5 do Modo %s!\n", modo_nome);
    printf("Digite seu nome (até 5 caracteres): ");

    // Limpa o buffer de entrada do scanf anterior e lê a string
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    // Lê o nome com fgets para controlar o tamanho
    fgets(nome_temp, MAX_NOME, stdin);
    nome_temp[strcspn(nome_temp, "\n")] = 0; // Remove a quebra de linha

    // Garante que o nome tenha no máximo 5 caracteres
    if (strlen(nome_temp) > 5) {
        nome_temp[5] = '\0';
    }

    // Substitui o último (menor) recorde
    strcpy(ranque[MAX_RANQUE - 1].nome, nome_temp);
    ranque[MAX_RANQUE - 1].pontuacao = nova_pontuacao;

    // Ordenação (Bubble Sort Decrescente)
    for (int i = 0; i < MAX_RANQUE - 1; i++) {
        for (int j = 0; j < MAX_RANQUE - 1 - i; j++) {
            if (ranque[j].pontuacao < ranque[j + 1].pontuacao) {
                Recorde temp = ranque[j];
                ranque[j] = ranque[j + 1];
                ranque[j + 1] = temp;
            }
        }
    }

    printf("\n? Recorde salvo com sucesso!\n");
}

void mostrar_ranque(Recorde ranque[], const char *modo) {
    system("clear || cls");
    printf("==============================\n");
    printf("?? RANQUE: MODO %s\n", modo);
    printf("==============================\n");
    printf("Pos | Nome | Pontuação\n");
    printf("----|------|----------\n");

    for (int i = 0; i < MAX_RANQUE; i++) {
        printf("%2d. | %-4s | %d\n", i + 1, ranque[i].nome, ranque[i].pontuacao);
    }

    printf("------------------------------\n");
    printf("\nPressione ENTER para voltar...");

    int c;
    while ((c = getchar()) != '\n' && c != EOF); // Limpa buffer
    getchar(); // Aguarda
}

// ----------------------------------------------------------------------
// Implementação: Menus de Interface (usando I/O padrão)
// ----------------------------------------------------------------------

void menu_principal() {
    int escolha;
    do {
        system("clear || cls");
        printf("==============================\n");
        printf("??? Jogo de Digitação (Typing Game)\n");
        printf("==============================\n");
        printf("1. Jogar\n");
        printf("2. Ranque\n");
        printf("3. Sair\n");
        printf("------------------------------\n");
        printf("Escolha a opção: ");

        if (scanf("%d", &escolha) != 1) {
            escolha = 0; // Trata entrada não numérica
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        }

        switch (escolha) {
            case 1:
                menu_jogar();
                break;
            case 2:
                menu_ranque();
                break;
            case 3:
                printf("\n?? Obrigado por jogar! Até a próxima.\n");
                return;
            default:
                printf("\n? Opção inválida. Pressione ENTER para continuar...");
                int c;
                while ((c = getchar()) != '\n' && c != EOF);
                getchar();
        }
    } while (escolha != 3);
}

void menu_jogar() {
    int escolha;
    do {
        system("clear || cls");
        printf("==============================\n");
        printf("?? Selecione a Dificuldade\n");
        printf("==============================\n");
        printf("1. Fácil\n");
        printf("2. Médio (Indisponível)\n");
        printf("3. Difícil (Indisponível)\n");
        printf("4. Voltar\n");
        printf("------------------------------\n");
        printf("Escolha o modo: ");

        if (scanf("%d", &escolha) != 1) {
            escolha = 0;
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        }

        switch (escolha) {
            case 1:
                // Inicia o ncurses para o jogo
                initscr();
                cbreak();
                noecho();
                curs_set(0);
                keypad(stdscr, TRUE);
                timeout(0);

                jogar_facil();

                // Encerra o ncurses após o jogo
                endwin();
                escolha = 4;
                break;
            case 2:
            case 3:
                printf("\n?? Modo em desenvolvimento! Pressione ENTER para continuar...");
                int c;
                while ((c = getchar()) != '\n' && c != EOF);
                getchar();
                break;
            case 4:
                return;
            default:
                printf("\n? Opção inválida. Pressione ENTER para continuar...");
                int c;
                while ((c = getchar()) != '\n' && c != EOF);
                getchar();
        }
    } while (escolha != 4);
}

void menu_ranque() {
    int escolha;
    do {
        system("clear || cls");
        printf("==============================\n");
        printf("?? Ranque de Pontuação\n");
        printf("==============================\n");
        printf("1. Ranque Fácil\n");
        printf("2. Ranque Médio\n");
        printf("3. Ranque Difícil\n");
        printf("4. Voltar\n");
        printf("------------------------------\n");
        printf("Escolha o ranque: ");

        if (scanf("%d", &escolha) != 1) {
            escolha = 0;
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        }

        switch (escolha) {
            case 1:
                mostrar_ranque(ranque_facil, "FÁCIL");
                break;
            case 2:
                mostrar_ranque(ranque_medio, "MÉDIO");
                break;
            case 3:
                mostrar_ranque(ranque_dificil, "DIFÍCIL");
                break;
            case 4:
                return;
            default:
                printf("\n? Opção inválida. Pressione ENTER para continuar...");
                int c;
                while ((c = getchar()) != '\n' && c != EOF);
                getchar();
        }
    } while (escolha != 4);
}

// ----------------------------------------------------------------------
// Implementação: Lógica de Jogo (NCURSES)
// ----------------------------------------------------------------------

void spawn_palavra(int index, int max_x) {
    if (palavras_ativas[index].ativa == 0) {
        int idx_palavra = rand() % NUM_PALAVRAS;
        palavras_ativas[index].palavra = (char *)PALAVRAS[idx_palavra];
        palavras_ativas[index].y_pos = 1;

        int p_len = strlen(palavras_ativas[index].palavra);
        // Garante que a palavra não saia da tela
        palavras_ativas[index].x_pos = 1 + rand() % (max_x - p_len - 2);

        palavras_ativas[index].progresso = 0;
        palavras_ativas[index].ativa = 1;
    }
}

void jogar_facil() {
    // Variáveis de Jogo
    int vidas = VIDAS_INICIAIS;
    long long pontuacao = 0;
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    // Variáveis de Dificuldade (Modo Fácil)
    double tempo_queda_ms = TEMPO_BASE_QUEDA_MS;
    // Mínimo de 9s (redução máxima de 6s, 6/15 * 625ms = 250ms. 625-250 = 375ms)
    double limite_queda_ms = TEMPO_BASE_QUEDA_MS - 250;

    double tempo_spawn_s = 10.0;
    int limite_spawn_s = 5;
    int palavras_simultaneas = 1;

    // Variáveis de Controle de Tempo
    clock_t ultima_queda = clock();
    clock_t ultimo_spawn = clock();

    // Variável de Entrada
    char buffer_entrada[50] = "";
    int buffer_len = 0;

    // Inicializa o array de palavras
    for (int i = 0; i < MAX_PALAVRAS_SIMULTANEAS; i++) {
        palavras_ativas[i].ativa = 0;
    }
    spawn_palavra(0, max_x);

    // --- Loop Principal do Jogo ---
    while (vidas > 0) {

        // 1. Limpa a tela
        erase();

        // 2. Exibe o HUD (Status)
        mvprintw(0, 1, "Vidas: %d | Pontos: %lld | Queda/Linha: %.2fs | Spawn: %.1fs | Multi: %d",
                 vidas, pontuacao, tempo_queda_ms / 1000.0, tempo_spawn_s, palavras_simultaneas);
        mvprintw(max_y - 2, 1, "-------------------------------------------------");
        mvprintw(max_y - 1, 1, "Digite: %s", buffer_entrada);

        // 3. Controle de Queda e Perda de Vida
        clock_t agora = clock();
        double tempo_passado_ms = (double)(agora - ultima_queda) * 1000.0 / CLOCKS_PER_SEC;

        if (tempo_passado_ms >= tempo_queda_ms) {
            ultima_queda = agora;
            for (int i = 0; i < palavras_simultaneas; i++) {
                if (palavras_ativas[i].ativa) {
                    palavras_ativas[i].y_pos++;

                    // Condição de Perda de Vida
                    if (palavras_ativas[i].y_pos >= max_y - 2) {
                        vidas--;
                        palavras_ativas[i].ativa = 0;
                        mvprintw(max_y - 3, 1, "? PERDEU! Palavra '%s' caiu.", palavras_ativas[i].palavra);

                        if (vidas > 0) {
                            spawn_palavra(i, max_x);
                        }
                    }
                }
            }
        }

        // 4. Controle de Spawn de Novas Palavras
        double tempo_spawn_passado_s = (double)(agora - ultimo_spawn) / CLOCKS_PER_SEC;

        if (tempo_spawn_passado_s >= tempo_spawn_s) {
            ultimo_spawn = agora;
            // Tenta spawnar uma palavra em um slot inativo
            for (int i = 0; i < palavras_simultaneas; i++) {
                if (palavras_ativas[i].ativa == 0) {
                    spawn_palavra(i, max_x);
                    break;
                }
            }
        }

        // 5. Desenha as Palavras Ativas
        for (int i = 0; i < palavras_simultaneas; i++) {
            if (palavras_ativas[i].ativa) {
                // Desenha a palavra
                mvprintw(palavras_ativas[i].y_pos, palavras_ativas[i].x_pos,
                         "%s", palavras_ativas[i].palavra);
            }
        }

        // 6. Entrada do Usuário (Non-blocking)
        int ch = getch();
        if (ch != ERR) {
            if (ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z') {
                // Adiciona o caractere ao buffer
                if (buffer_len < 49) {
                    buffer_entrada[buffer_len++] = (char)ch;
                    buffer_entrada[buffer_len] = '\0';
                }

                // Verifica o buffer contra as palavras ativas
                for (int i = 0; i < palavras_simultaneas; i++) {
                    if (palavras_ativas[i].ativa) {
                        // Verifica se o buffer corresponde à palavra completa
                        if (strcmp(buffer_entrada, palavras_ativas[i].palavra) == 0) {

                            int len = strlen(palavras_ativas[i].palavra);
                            pontuacao += len * 10;

                            // --------------------- REGRAS FÁCIL ---------------------
                            // a) Queda (500 pts = -1s, máx -6s => -250ms)
                            if (pontuacao % 500 == 0 && tempo_queda_ms > limite_queda_ms) {
                                tempo_queda_ms -= 41.66 * 6; // Ajuste para 1 segundo total de redução
                                if (tempo_queda_ms < limite_queda_ms) tempo_queda_ms = limite_queda_ms;
                            }
                            // b) Spawn (150 pts = -1s, máx -5s)
                            if (pontuacao % 150 == 0 && tempo_spawn_s > limite_spawn_s) {
                                tempo_spawn_s -= 1.0;
                            }
                            // c) Multi-palavra (após limite de spawn)
                            if (tempo_spawn_s <= limite_spawn_s && pontuacao > 0 && pontuacao % 500 == 0 && palavras_simultaneas < MAX_PALAVRAS_SIMULTANEAS) {
                                palavras_simultaneas++;
                            }
                            // --------------------------------------------------------

                            // Finaliza a palavra e limpa o buffer
                            palavras_ativas[i].ativa = 0;
                            buffer_len = 0;
                            buffer_entrada[0] = '\0';

                            spawn_palavra(i, max_x);
                            break;
                        }
                    }
                }

            } else if (ch == 127 || ch == KEY_BACKSPACE || ch == '\b') {
                // Backspace
                if (buffer_len > 0) {
                    buffer_entrada[--buffer_len] = '\0';
                }
            } else if (ch == ' ' || ch == '\n') {
                // Limpa o buffer ao digitar espaço ou Enter
                buffer_len = 0;
                buffer_entrada[0] = '\0';
            }
        }

        // 7. Atualiza a tela e espera
        refresh();
        usleep(50000); // 50ms de pausa
    }
    // --- Fim do Loop do Jogo ---

    // Exibição de Fim de Jogo
    erase();
    mvprintw(max_y / 2 - 2, max_x / 2 - 10, "============================");
    mvprintw(max_y / 2 - 1, max_x / 2 - 10, "        FIM DE JOGO!        ");
    mvprintw(max_y / 2, max_x / 2 - 10, "============================");
    mvprintw(max_y / 2 + 1, max_x / 2 - 10, "Pontuação Final: %lld", pontuacao);

    refresh();
    sleep(2);

    // Atualiza o Ranque (passando a pontuação)
    endwin(); // Encerra ncurses temporariamente para usar I/O padrão
    atualizar_ranque(ranque_facil, (int)pontuacao, "FÁCIL");

    printf("\nPressione ENTER para voltar ao menu...");

    // Garante que o buffer de entrada esteja limpo antes de esperar
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    getchar(); // Espera o ENTER final
}

