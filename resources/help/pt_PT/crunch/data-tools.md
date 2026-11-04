# Ferramentas de dados

<!-- keywords: dados, separador dados, texto para colunas, dividir, separador, vírgula, tabulação, largura fixa, remover duplicados, linhas repetidas, únicos, ficheiro de texto, csv, a partir de texto, colunas, delimitador, aspas -->

O separador **Dados** tem as ferramentas que atuam sobre uma lista de valores: obter texto de um ficheiro, ordenar uma lista, dividir texto em colunas e eliminar linhas repetidas.

## Obter Dados de Texto/CSV

**De Texto/CSV** abre um ficheiro de texto (.csv, .tsv ou .txt). Tem o mesmo efeito que **Abrir Ficheiro de Texto** no separador Ficheiro. O JeffCrunch pergunta como está o ficheiro dividido em colunas e que tipo de texto contém, e depois apresenta-o como um novo livro.

## Ordenar e filtrar

O grupo **Ordenar e Filtrar** tem os mesmos comandos do menu **Ordenar e Filtrar Linhas** no separador Base. Consulte [Ordenar e filtrar](sort-filter).

## Texto para Colunas

Use esta opção quando uma coluna tem vários pedaços de texto que pretende ter em colunas separadas, como "Silva, Ana, 31".

1. Selecione as células de **uma** coluna.
2. Escolha **Texto para Colunas** no separador **Dados**.
3. Escolha a forma de dividir o texto:
   - **Dividir num separador**: marque os sinais que separam os pedaços: **Tabulação**, **Ponto e vírgula**, **Vírgula**, **Espaço** ou **Outro**, onde escreve o seu próprio sinal. **Tratar separadores consecutivos como um só** evita que duas vírgulas seguidas criem um pedaço vazio. **Texto entre aspas** indica o sinal que envolve um pedaço que não deve ser dividido, como "Lee, Bo". Dentro das aspas, dois sinais seguidos valem como um só.
   - **Dividir em posições fixas**: escreva as posições onde começa uma nova coluna, contando os carateres a partir de 1. Por exemplo, 5, 12 inicia uma nova coluna no quinto e no décimo segundo carácter.
4. Escolha o que é cada pedaço: **Geral** lê números e datas como se os tivesse escrito, e **Texto** mantém cada pedaço tal como está, para que "007" continue a ser "007".
5. Em **Colocar os pedaços a partir de**, escreva a primeira célula para os pedaços. Começa com a primeira célula que selecionou.
6. A caixa por baixo das opções mostra as primeiras linhas divididas como escolheu. Prima **OK**.

Os pedaços vão para as células à direita da primeira célula e substituem o que nelas existia. Texto para Colunas é um passo que pode ser anulado.

## Remover Duplicados

Use esta opção para manter apenas a primeira de cada grupo de linhas repetidas.

1. Selecione as células da lista, ou uma célula dentro dela. O JeffCrunch usa todo o bloco de dados à volta de uma única célula.
2. Escolha **Remover Duplicados**.
3. Marque **Os meus dados têm uma linha de cabeçalho** se a primeira linha tiver nomes. Essa linha mantém-se e não é comparada.
4. Marque as colunas que têm de coincidir para que duas linhas sejam consideradas repetidas. Duas linhas só se repetem quando todas as colunas marcadas são iguais. As letras maiúsculas e minúsculas são tratadas como iguais, e uma fórmula conta pelo valor que devolve.
5. Prima **OK**. A caixa de diálogo indica quantas linhas foram removidas e quantas restam.

As linhas abaixo de uma linha removida sobem dentro das células selecionadas, com os respetivos formatos e fórmulas. As células abaixo da seleção não se deslocam. As linhas vazias que ficam no fim da seleção são limpas. Remover Duplicados é um passo que pode ser anulado.
