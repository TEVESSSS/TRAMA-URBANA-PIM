/* SISTEMA DE GESTAO DE PEDIDOS E ESTOQUE - PIM UNIP (ADS)
   Compilar: gcc sistema_pedidos_curto.c -o sistema   |   Executar: ./sistema
   Login do administrador: admin@loja.com / admin123 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct { int id; char nome[50]; char email[50]; char senha[50]; } Usuario;
typedef struct { int id; char nome[50]; float preco; int estoque, estoqueInicial, entradas, saidas; } Produto;
typedef struct {
    int id, idUsuario;
    char data[30], status[20];          /* status: Concluido ou Cancelado */
    float valorTotal;
    int qtdItens, idProduto[10], quantidade[10];
} Pedido;

Usuario usuarios[50];  int totalUsuarios = 0;
Produto produtos[100]; int totalProdutos = 0, proximoIdProduto = 1;
Pedido pedidos[100];   int totalPedidos = 0;
Pedido carrinho;                        /* pedido que esta sendo montado */
/* ---------- leitura segura (nao trava com entrada errada) ---------- */
int limparLinha() {
    int c = getchar();
    while (c != '\n' && c != EOF) c = getchar();
    return 0;
}
int lerInt() {
    int n = 0, r = scanf("%d", &n);
    while (r != 1) {
        if (r == EOF) return 0;
        limparLinha();
        printf("Valor invalido! Digite apenas numeros: ");
        r = scanf("%d", &n);
    }
    limparLinha();
    return n;
}
int lerTexto(char d[]) {
    d[0] = '\0';
    scanf(" %49[^\n]", d);
    limparLinha();
    return strlen(d);
}
float lerFloat() {                      /* aceita 59.90 ou 59,90 */
    char t[50];
    int i;
    lerTexto(t);
    for (i = 0; t[i] != '\0'; i++) if (t[i] == ',') t[i] = '.';
    return atof(t);
}
/* ---------- funcoes auxiliares ---------- */
int linha() { printf("------------------------------------------------------------\n"); return 0; }
int paraMinusculo(char origem[], char destino[]) {
    int i;
    for (i = 0; origem[i] != '\0'; i++) {
        destino[i] = origem[i];
        if (destino[i] >= 'A' && destino[i] <= 'Z') destino[i] = destino[i] + 32;
    }
    destino[i] = '\0';
    return i;
}
/* RNF02: cifra de Cesar (cada letra anda 3 posicoes) - versao didatica */
int cifrar(char texto[]) {
    int i;
    for (i = 0; texto[i] != '\0'; i++) texto[i] = texto[i] + 3;
    return 1;
}
/* RNF01: mostra quanto tempo a operacao levou */
int tempo(clock_t inicio) {
    printf("Tempo de resposta: %.3f s (meta: menos de 2 s)\n", (double)(clock() - inicio) / CLOCKS_PER_SEC);
    return 0;
}
/* RN02: formato valido = texto@dominio.com */
int emailValido(char e[]) {
    int i, arroba = -1, ponto = -1, tam = strlen(e);
    for (i = 0; i < tam; i++) {
        if (e[i] == ' ') return 0;
        if (e[i] == '@' && arroba != -1) return 0;
        if (e[i] == '@') arroba = i;
        if (e[i] == '.' && arroba != -1) ponto = i;
    }
    if (arroba >= 1 && ponto >= arroba + 2 && ponto < tam - 1) return 1;
    return 0;
}
int buscarUsuario(char email[]) {
    int i;
    for (i = 0; i < totalUsuarios; i++) if (strcmp(usuarios[i].email, email) == 0) return i;
    return -1;
}
int buscarProduto(int id) {
    int i;
    for (i = 0; i < totalProdutos; i++) if (produtos[i].id == id) return i;
    return -1;
}
/* RN03: total = preco unitario x quantidade */
float calcularTotal(float precoUnitario, int quantidade) { return precoUnitario * quantidade; }
/* pede e-mail valido e unico (RN02). "ignorado" = indice do proprio usuario
   na atualizacao. Digitar 0 cancela. */
int pedirEmail(char email[], int ignorado) {
    int idx;
    while (1) {
        printf("E-mail (0 para cancelar): ");
        if (lerTexto(email) == 0 || strcmp(email, "0") == 0) return 0;
        paraMinusculo(email, email);
        idx = buscarUsuario(email);
        if (emailValido(email) == 0) printf("E-mail invalido! Exemplo: nome@dominio.com\n");
        else if (idx != -1 && idx != ignorado) printf("Este e-mail ja esta cadastrado (RN02)!\n");
        else return 1;
    }
}
int pedirSenha(char senha[]) {
    while (1) {
        printf("Senha (minimo 4 caracteres, 0 para cancelar): ");
        if (lerTexto(senha) == 0 || strcmp(senha, "0") == 0) return 0;
        if (strlen(senha) >= 4) return 1;
        printf("Senha muito curta!\n");
    }
}
/* ---------- RF01: usuarios (incluir, consultar, atualizar) ---------- */
int cadastrarUsuario() {
    Usuario novo;
    printf("\n=== CADASTRO DE USUARIO ===\n");
    if (totalUsuarios >= 50) { printf("Limite de usuarios atingido!\n"); return 0; }
    printf("Nome: ");
    if (lerTexto(novo.nome) == 0 || pedirEmail(novo.email, -1) == 0 || pedirSenha(novo.senha) == 0) { printf("Cadastro cancelado.\n"); return 0; }
    cifrar(novo.senha);
    novo.id = totalUsuarios + 1;
    usuarios[totalUsuarios] = novo;
    totalUsuarios++;
    printf("Cadastrado com sucesso! Seu ID e %d.\n", novo.id);
    return 1;
}
/* u = posicao do usuario logado (0 = administrador) */
int menuUsuarios(int u) {
    int op = -1;
    char texto[50];
    while (op != 0) {
        printf("\n=== USUARIOS ===\n1 - Ver meus dados\n2 - Alterar nome\n3 - Alterar e-mail\n"
               "4 - Alterar senha\n0 - Voltar\nOpcao: ");
        op = lerInt();
        if (op == 1) printf("ID: %d | Nome: %s | E-mail: %s\n", usuarios[u].id, usuarios[u].nome, usuarios[u].email);
        else if (op == 2) {
            printf("Novo nome: ");
            if (lerTexto(texto) > 0) { strcpy(usuarios[u].nome, texto); printf("Atualizado!\n"); }
        } else if (op == 3) {
            if (pedirEmail(texto, u)) { strcpy(usuarios[u].email, texto); printf("Atualizado!\n"); }
        } else if (op == 4) {
            if (pedirSenha(texto)) { cifrar(texto); strcpy(usuarios[u].senha, texto); printf("Atualizado!\n"); }
        } else if (op != 0) printf("Opcao invalida!\n");
    }
    return 0;
}
/* ---------- RF02: consultar produtos ---------- */
/* busca vazia ("") mostra todos; senao filtra pelo nome */
int listarProdutos(char busca[]) {
    int i, achou = 0;
    char buscaMin[50], nomeMin[50];
    clock_t inicio = clock();
    paraMinusculo(busca, buscaMin);
    printf("\n%-5s %-24s %12s %8s  %s\n", "ID", "Produto", "Preco", "Estoque", "Situacao");
    linha();
    for (i = 0; i < totalProdutos; i++) {
        paraMinusculo(produtos[i].nome, nomeMin);
        if (strstr(nomeMin, buscaMin) != NULL) {
            printf("%-5d %-24s R$ %9.2f %7d  ", produtos[i].id, produtos[i].nome, produtos[i].preco, produtos[i].estoque);
            if (produtos[i].estoque > 0) printf("Disponivel\n"); else printf("Indisponivel\n");
            achou++;
        }
    }
    if (achou == 0) printf("Nenhum produto encontrado.\n");
    linha();
    tempo(inicio);
    return achou;
}
int consultarProdutos() {
    char busca[50];
    printf("\n=== PRODUTOS ===\nDigite parte do nome (ou 0 para listar todos): ");
    lerTexto(busca);
    if (strcmp(busca, "0") == 0) listarProdutos("");
    else listarProdutos(busca);
    return 0;
}
/* ---------- RF03: realizar pedido ---------- */
int realizarPedido(int u) {
    int id, qtd, i, p, pos, jaNo;
    float total = 0, sub;
    time_t agora = time(NULL);                       /* data automatica */
    struct tm *t = localtime(&agora);
    printf("\n=== REALIZAR PEDIDO ===\n");
    if (totalPedidos >= 100 || totalProdutos == 0) { printf("Sem produtos cadastrados ou limite de pedidos atingido!\n"); return 0; }
    carrinho.qtdItens = 0;
    listarProdutos("");
    printf("\nID do produto (0 para finalizar): ");
    id = lerInt();
    while (id != 0) {
        p = buscarProduto(id);
        printf("Quantidade: ");
        qtd = lerInt();
        pos = -1;                                    /* ja esta no pedido? */
        jaNo = 0;
        for (i = 0; i < carrinho.qtdItens; i++)
            if (carrinho.idProduto[i] == id) { pos = i; jaNo = carrinho.quantidade[i]; }
        if (p == -1) printf("Produto nao encontrado!\n");
        else if (qtd <= 0) printf("Quantidade invalida!\n");
        else if (qtd + jaNo > produtos[p].estoque)                                 /* RN01 */
            printf("Estoque insuficiente (RN01)! Disponivel: %d | No pedido: %d\n", produtos[p].estoque, jaNo);
        else if (pos == -1 && carrinho.qtdItens >= 10) printf("Limite de 10 produtos por pedido!\n");
        else {
            if (pos == -1) {
                pos = carrinho.qtdItens;
                carrinho.idProduto[pos] = id;
                carrinho.quantidade[pos] = 0;
                carrinho.qtdItens++;
            }
            carrinho.quantidade[pos] = carrinho.quantidade[pos] + qtd;
            printf("Adicionado: %d x %s\n", qtd, produtos[p].nome);
        }
        printf("\nID do produto (0 para finalizar): ");
        id = lerInt();
    }
    if (carrinho.qtdItens == 0) { printf("Pedido vazio. Nada foi registrado.\n"); return 0; }
    printf("\n=== RESUMO DO PEDIDO ===\n");
    for (i = 0; i < carrinho.qtdItens; i++) {
        p = buscarProduto(carrinho.idProduto[i]);
        sub = calcularTotal(produtos[p].preco, carrinho.quantidade[i]);
        printf("%-24s %3d x R$ %8.2f = R$ %9.2f\n", produtos[p].nome, carrinho.quantidade[i], produtos[p].preco, sub);
        total = total + sub;
    }
    linha();
    printf("TOTAL (RN03): R$ %.2f\n1 - Confirmar pedido | outro numero - Cancelar\nOpcao: ", total);
    carrinho.id = totalPedidos + 1;
    carrinho.idUsuario = usuarios[u].id;
    carrinho.valorTotal = total;
    sprintf(carrinho.data, "%02d/%02d/%04d", t->tm_mday, t->tm_mon + 1, t->tm_year + 1900);
    if (lerInt() == 1) {
        for (i = 0; i < carrinho.qtdItens; i++) {    /* baixa no estoque */
            p = buscarProduto(carrinho.idProduto[i]);
            produtos[p].estoque = produtos[p].estoque - carrinho.quantidade[i];
            produtos[p].saidas = produtos[p].saidas + carrinho.quantidade[i];
        }
        strcpy(carrinho.status, "Concluido");
        printf("Pedido %d CONCLUIDO!\n", carrinho.id);
    } else {
        strcpy(carrinho.status, "Cancelado");
        printf("Pedido %d CANCELADO. Estoque mantido.\n", carrinho.id);
    }
    pedidos[totalPedidos] = carrinho;
    totalPedidos++;
    return 1;
}
/* ---------- RF04: relatorios (somente administrador) ---------- */
int relatorios() {
    int op = -1, i, n;
    float soma;
    clock_t inicio;
    while (op != 0) {
        printf("\n=== RELATORIOS ===\n1 - Vendas e status dos pedidos\n2 - Movimentacao de estoque\n0 - Voltar\nOpcao: ");
        op = lerInt();
        inicio = clock();
        n = 0;
        soma = 0;
        if (op == 1) {
            printf("\n%-8s %-12s %-20s %-12s %12s\n", "Pedido", "Data", "Cliente", "Status", "Total");
            linha();
            for (i = 0; i < totalPedidos; i++) {
                printf("%-8d %-12s %-20s %-12s R$ %9.2f\n", pedidos[i].id, pedidos[i].data,
                       usuarios[pedidos[i].idUsuario - 1].nome, pedidos[i].status, pedidos[i].valorTotal);
                if (strcmp(pedidos[i].status, "Concluido") == 0) { n++; soma = soma + pedidos[i].valorTotal; }
            }
            linha();
            printf("Concluidos: %d | Cancelados: %d | Faturamento: R$ %.2f\n", n, totalPedidos - n, soma);
        } else if (op == 2) {
            printf("\n%-5s %-22s %8s %8s %8s %8s\n", "ID", "Produto", "Inicial", "Entradas", "Saidas", "Saldo");
            linha();
            for (i = 0; i < totalProdutos; i++)
                printf("%-5d %-22s %8d %8d %8d %8d\n", produtos[i].id, produtos[i].nome, produtos[i].estoqueInicial,
                       produtos[i].entradas, produtos[i].saidas, produtos[i].estoque);
            linha();
        } else if (op != 0) printf("Opcao invalida!\n");
        if (op == 1 || op == 2) tempo(inicio);
    }
    return 0;
}
/* ---------- gestao de produtos (admin): cadastrar, remover e repor estoque ---------- */
int gerenciarProdutos() {
    int op = -1, i, p, qtd;
    Produto novo;
    while (op != 0) {
        printf("\n=== GERENCIAR PRODUTOS ===\n1 - Cadastrar produto\n2 - Remover produto\n3 - Repor estoque\n0 - Voltar\nOpcao: ");
        op = lerInt();
        if (op == 1 && totalProdutos >= 100) printf("Limite de produtos atingido!\n");
        else if (op == 1) {
            printf("Nome do produto (ex: Camiseta Preta M): ");
            lerTexto(novo.nome);
            printf("Preco (ex: 59,90): ");
            novo.preco = lerFloat();
            printf("Quantidade em estoque: ");
            novo.estoque = lerInt();
            if (novo.preco <= 0 || novo.estoque < 0 || strlen(novo.nome) == 0) printf("Dados invalidos! Produto nao cadastrado.\n");
            else {
                novo.id = proximoIdProduto;
                novo.estoqueInicial = novo.estoque;
                novo.entradas = 0;
                novo.saidas = 0;
                produtos[totalProdutos] = novo;
                totalProdutos++;
                proximoIdProduto++;
                printf("Produto cadastrado! ID: %d\n", novo.id);
            }
        } else if (op == 2 || op == 3) {
            listarProdutos("");
            printf("ID do produto: ");
            p = buscarProduto(lerInt());
            if (p == -1) printf("Produto nao encontrado!\n");
            else if (op == 2) {
                printf("Remover %s? 1 - Sim | outro numero - Nao\nOpcao: ", produtos[p].nome);
                if (lerInt() == 1) {
                    for (i = p; i < totalProdutos - 1; i++) produtos[i] = produtos[i + 1];
                    totalProdutos--;
                    printf("Produto removido!\n");
                }
            } else {
                printf("Quantidade a repor: ");
                qtd = lerInt();
                if (qtd <= 0) printf("Quantidade invalida!\n");
                else {
                    produtos[p].estoque = produtos[p].estoque + qtd;
                    produtos[p].entradas = produtos[p].entradas + qtd;
                    printf("Estoque de %s agora: %d\n", produtos[p].nome, produtos[p].estoque);
                }
            }
        } else if (op != 0) printf("Opcao invalida!\n");
    }
    return 0;
}
/* ---------- login (RNF02) e menus ---------- */
int fazerLogin() {
    char email[50], senha[50];
    int tentativa, idx;
    for (tentativa = 1; tentativa <= 3; tentativa++) {
        printf("\nE-mail: ");
        if (lerTexto(email) == 0) return -1;
        paraMinusculo(email, email);
        printf("Senha: ");
        if (lerTexto(senha) == 0) return -1;
        cifrar(senha);
        idx = buscarUsuario(email);
        if (idx != -1 && strcmp(usuarios[idx].senha, senha) == 0) {
            printf("Bem-vindo(a), %s!\n", usuarios[idx].nome);
            return idx;
        }
        printf("E-mail ou senha incorretos (tentativa %d de 3).\n", tentativa);
    }
    return -1;
}
/* devolve o usuario logado, ou -1 para sair do sistema */
int telaInicial() {
    int op = -1, idx;
    while (op != 0) {
        printf("\n===== SISTEMA DE PEDIDOS E ESTOQUE =====\n1 - Entrar\n2 - Cadastrar-se\n0 - Sair do sistema\nOpcao: ");
        op = lerInt();
        if (op == 1) {
            idx = fazerLogin();
            if (idx != -1) return idx;
        } else if (op == 2) cadastrarUsuario();
        else if (op != 0) printf("Opcao invalida!\n");
    }
    return -1;
}
int menuPrincipal(int u) {
    int op = -1;
    while (op != 0) {
        printf("\n===== MENU PRINCIPAL (%s) =====\n1 - Usuarios\n2 - Consultar produtos\n3 - Realizar pedido\n"
               "4 - Relatorios (admin)\n5 - Gerenciar produtos (admin)\n0 - Sair da conta\nOpcao: ", usuarios[u].nome);
        op = lerInt();
        if (op == 1) menuUsuarios(u);
        else if (op == 2) consultarProdutos();
        else if (op == 3) realizarPedido(u);
        else if ((op == 4 || op == 5) && u != 0) printf("Acesso negado: apenas o administrador.\n");
        else if (op == 4) relatorios();
        else if (op == 5) gerenciarProdutos();
        else if (op != 0) printf("Opcao invalida!\n");
    }
    return 0;
}

int main() {
    int idx;
    Usuario admin = {1, "Administrador", "admin@loja.com", "admin123"};    /* administrador */
    cifrar(admin.senha);
    usuarios[0] = admin;
    totalUsuarios = 1;
    idx = telaInicial();
    while (idx != -1) {
        menuPrincipal(idx);
        idx = telaInicial();
    }
    printf("\nSistema encerrado. Ate logo!\n");
    return 0;
}