# Ferramentas de dados
<!-- keywords: dados, guia dados, texto para colunas, dividir, separador, vírgula, tabulação, largura fixa, remover duplicatas, linhas repetidas, exclusivos, arquivo de texto, csv, de texto, colunas, delimitador, aspas -->

A guia **Dados** tem as ferramentas que trabalham com uma lista de valores: obter texto de um arquivo, colocar uma lista em ordem, dividir um texto em colunas e remover linhas repetidas.

## De Texto/CSV

**De Texto/CSV** abre um arquivo de texto (.csv, .tsv ou .txt). É o mesmo que **Abrir Arquivo de Texto** na guia Arquivo. O JeffCrunch pergunta como o arquivo está dividido em colunas e que tipo de texto ele contém, e então o mostra como uma nova pasta de trabalho.

## Classificar e filtrar

O grupo **Classificar e Filtrar** tem os mesmos comandos do menu **Classificar e Filtrar** na guia Página Inicial. Veja [Classificar e filtrar](sort-filter).

## Texto para Colunas

Use este recurso quando uma coluna tiver vários trechos de texto que você quer em colunas separadas, como "Silva, Ana, 31".

1. Selecione as células de **uma** coluna.
2. Escolha **Texto para Colunas** na guia **Dados**.
3. Escolha como dividir o texto:
   - **Dividir em um separador**: marque os sinais que ficam entre os trechos: **Tabulação**, **Ponto e vírgula**, **Vírgula**, **Espaço** ou **Outro**, em que você digita o seu próprio sinal. **Tratar separadores consecutivos como um só** evita que duas vírgulas seguidas criem um trecho vazio. **Texto entre aspas** indica o sinal que envolve um trecho que não deve ser dividido, como "Lee, Bo". Dentro das aspas, dois sinais seguidos valem como um só.
   - **Dividir em posições fixas**: digite as posições em que uma nova coluna começa, contando os caracteres a partir de 1. Por exemplo, 5, 12 inicia uma nova coluna no quinto e no décimo segundo caractere.
4. Escolha o que cada trecho é: **Geral** lê números e datas como se você os tivesse digitado, e **Texto** mantém cada trecho como está, de modo que "007" continua "007".
5. Em **Colocar os trechos a partir de**, digite a primeira célula para os trechos. Ela começa como a primeira célula que você selecionou.
6. A caixa sob as opções mostra as primeiras linhas divididas da forma que você escolheu. Pressione **OK**.

Os trechos vão para as células à direita da primeira célula e substituem o que havia nelas. Texto para Colunas é uma etapa que você pode desfazer.

## Remover Duplicatas

Use este recurso para manter apenas a primeira de cada grupo de linhas repetidas.

1. Selecione as células da lista, ou uma célula dela. O JeffCrunch usa todo o bloco de dados ao redor de uma única célula.
2. Escolha **Remover Duplicatas**.
3. Marque **Meus dados têm linha de cabeçalho** se a primeira linha tiver nomes. Essa linha permanece e não é comparada.
4. Marque as colunas que devem coincidir para que duas linhas sejam consideradas repetidas. Duas linhas se repetem somente quando todas as colunas marcadas são iguais. Letras maiúsculas e minúsculas são tratadas como iguais, e uma fórmula conta pelo valor que produz.
5. Pressione **OK**. A caixa de diálogo informa quantas linhas foram removidas e quantas restaram.

As linhas abaixo de uma linha removida sobem dentro das células selecionadas, com suas formatações e fórmulas. As células abaixo da seleção não se movem. As linhas vazias que sobram no fim da seleção são limpas. Remover Duplicatas é uma etapa que você pode desfazer.
