# Ferramentas de Dados
<!-- keywords: dados, separador dados, texto para colunas, dividir, separador, vírgula, tabulação, largura fixa, remover duplicados, atingir objetivo, subtotal, totais, filtro avançado, condições, validação de dados, assinalar dados inválidos, círculo vermelho, limpar assinalações de validação, consolidar, combinar intervalos, permitir, lista, lista pendente, mensagem de entrada, alerta de erro, linhas repetidas, único, ficheiro de texto, csv, de texto, colunas, delimitador, aspas -->

O separador **Dados** tem as ferramentas que trabalham sobre uma lista de valores: obter texto de um ficheiro, ordenar uma lista, dividir texto em colunas e eliminar linhas repetidas.

## De Texto/CSV

**De Texto/CSV** abre um ficheiro de texto (.csv, .tsv ou .txt). Funciona da mesma forma que **Abrir Ficheiro de Texto** no separador Ficheiro. O JeffCrunch pergunta como o ficheiro está dividido em colunas e que tipo de texto contém, e depois mostra-o como um novo livro.

## Ordenar e filtrar

O grupo **Ordenar e Filtrar** tem os mesmos comandos do menu **Ordenar e Filtrar Linhas** no separador Base. Veja [Ordenar e filtrar](sort-filter).

## Texto para Colunas

Use esta opção quando uma coluna tem vários pedaços de texto que pretende ter em colunas separadas, como "Silva, Ana, 31".

1. Selecione as células de **uma** coluna.
2. Escolha **Texto para Colunas** no separador **Dados**.
3. Escolha a forma de dividir o texto:
   - **Dividir num separador**: marque os carateres que existem entre os pedaços: **Tabulação**, **Ponto e vírgula**, **Vírgula**, **Espaço** ou **Outro**, onde escreve o seu próprio carácter. **Tratar separadores consecutivos como um só** evita que duas vírgulas seguidas criem um pedaço vazio. **Texto entre aspas** indica o carácter que envolve um pedaço que não deve ser dividido, como "Lee, Bo". Dentro das aspas, dois carateres seguidos valem por um só.
   - **Dividir em posições fixas**: escreva as posições onde começa uma nova coluna, contando os carateres a partir de 1. Por exemplo, 5, 12 começa uma nova coluna no quinto e no décimo segundo carácter.
4. Escolha o que é cada pedaço: **Geral** lê números e datas como se os tivesse escrito, e **Texto** mantém cada pedaço tal como está, para que "007" continue a ser "007".
5. Em **Colocar as partes a partir de**, escreva a primeira célula para as partes. Inicialmente é a primeira célula que selecionou.
6. A caixa por baixo das opções mostra as primeiras linhas divididas da forma escolhida. Prima **OK**.

As partes são colocadas nas células à direita da primeira célula e substituem o que lá estava. Texto para Colunas é um passo que pode anular.

## Remover Duplicados

Use esta opção para manter apenas a primeira de cada grupo de linhas repetidas.

1. Selecione as células da lista ou uma célula dentro dela. O JeffCrunch usa o bloco de dados completo à volta de uma única célula.
2. Escolha **Remover Duplicados**.
3. Marque **Os meus dados têm linha de cabeçalho** se a primeira linha tiver nomes. Essa linha mantém-se e não é comparada.
4. Marque as colunas que têm de coincidir para que duas linhas sejam consideradas repetidas. Duas linhas só são repetidas quando todas as colunas marcadas são iguais. Letras maiúsculas e minúsculas são tratadas como iguais, e uma fórmula conta pelo valor que devolve.
5. Prima **OK**. A caixa de diálogo indica quantas linhas foram removidas e quantas restam.

As linhas abaixo de uma linha removida sobem dentro das células selecionadas, com os respetivos formatos e fórmulas. As células abaixo da seleção não se deslocam. As linhas vazias que ficam no fim da seleção são limpas. Remover Duplicados é um passo que pode anular.

## Atingir Objetivo

Use esta opção quando sabe o resultado que pretende, mas não o número que o produz. Por exemplo, uma célula de pagamento tem uma fórmula que usa uma taxa de juro, e pretende que o pagamento seja 500. O Atingir Objetivo experimenta números na célula da taxa até o pagamento ser 500.

1. Escolha **Atingir Objetivo** no separador **Dados**.
2. Em **Célula com a fórmula**, escreva a célula que contém a fórmula, como B3. Inicialmente é a célula que selecionou.
3. Em **Valor pretendido**, escreva o resultado que pretende que a fórmula devolva.
4. Em **Célula a alterar**, escreva a célula que o Atingir Objetivo pode alterar, como B1. Tem de conter um número, não uma fórmula.
5. Prima **OK**.

Se o Atingir Objetivo encontrar um número, este fica na célula a alterar, e a barra de estado indica qual é. Se não o conseguir, nada é alterado e é informado disso. Experimente outro número inicial na célula a alterar e confirme que a fórmula usa mesmo essa célula. Atingir Objetivo é um passo que pode anular.

## Subtotal

Use esta opção para colocar uma linha de totais por baixo de cada grupo de linhas que têm a mesma entrada numa coluna, e um total geral no fim. A lista precisa de uma linha de cabeçalho e deve estar ordenada pela coluna que agrupa, para que as entradas iguais fiquem juntas.

1. Selecione uma célula da lista.
2. Escolha **Subtotal** no grupo **Estrutura de Tópicos** do separador **Dados**.
3. Em **A cada alteração em**, escolha a coluna que agrupa as linhas, como Região.
4. Em **Utilizar função**, escolha o que calcular: **Soma**, **Contar**, **Média**, **Máx.**, **Mín.** e outras.
5. Em **Adicionar subtotal a**, marque cada coluna a totalizar.
6. Deixe **Substituir subtotais atuais** marcado para trocar os subtotais que já existam. Prima **OK**.

Cada linha de total diz "Total de Este" (ou a entrada a que se refere) e contém uma fórmula SUBTOTAL, por isso ignora as outras linhas de total. As linhas de cada grupo ficam agrupadas, pelo que pode usar os botões 1, 2 e 3 junto às linhas para mostrar apenas os totais. Para remover os totais, abra **Subtotal** novamente e prima **Remover Tudo**. Adicionar ou remover subtotais é um passo que pode anular.

## Filtro Avançado

Os botões de filtro escolhem linhas por uma coluna de cada vez. **Avançado** (no grupo **Ordenar e Filtrar**) permite escrever as condições na folha, para que possam ser tão longas quanto necessário.

1. Junto à lista, copie os cabeçalhos das colunas que pretende testar para uma linha de células vazias.
2. Por baixo de cada cabeçalho, escreva uma condição. As condições na **mesma linha** têm de ser todas verdadeiras (isto é "e"). As condições em **linhas diferentes** são alternativas: uma linha da lista só precisa de cumprir uma dessas linhas (isto é "ou").
3. Escolha **Avançado**. Confirme que o **Intervalo da lista** é a lista com a respetiva linha de cabeçalho, e escreva as células dos cabeçalhos e das condições em **Intervalo das condições**.
4. Escolha **Ocultar as outras linhas da lista** ou **Copiar as linhas para outro local** e escreva a primeira célula do local, como H1. Marque **Omitir linhas que repetem uma anterior** para copiar apenas uma de cada.
5. Prima **OK**.

Como escrever uma condição:

- Uma palavra, como **An**, escolhe texto que começa por essa palavra. Letras maiúsculas e minúsculas são tratadas como iguais. **?** representa qualquer carácter e ***** qualquer sequência de carateres.
- Um número escolhe células que lhe são iguais.
- **=Ana** escolhe texto que é exatamente Ana. **=** sozinho escolhe células vazias.
- **>**, **>=**, **<**, **<=** e **<>** (diferente de) antes de um número ou de uma palavra comparam com esse valor, como **>25**. **<>** sozinho escolhe células que não estão vazias.

Uma linha de condições sem nada escrito escolhe todas as linhas. Para mostrar novamente as linhas ocultas, escolha **Limpar**. Uma cópia é feita de imediato e não muda quando a lista mudar. Filtro Avançado é um passo que pode anular.

## Validação de Dados

Use esta opção para definir o que pode ser escrito em algumas células, para que uma entrada errada seja impedida ou questionada.

1. Selecione as células.
2. Escolha **Validação de Dados** no grupo **Ferramentas de Dados** do separador **Dados**.
3. No separador **Definições**, escolha o que **Permitir**: **Qualquer valor** (sem regra), **Número inteiro**, **Decimal**, **Lista**, **Data**, **Hora**, **Comprimento do texto** ou **Personalizado**.
4. Para um número, data, hora ou comprimento de texto, escolha **Dados** (**entre**, **não está entre**, **igual a**, e assim por diante) e escreva os limites. Um limite pode ser um valor ou uma fórmula que começa por =.
5. Para uma **Lista**, escreva as opções separadas por vírgulas, como Vermelho,Verde,Azul, ou escreva um intervalo de células depois de =, como =$A$1:$A$5. A célula passa a mostrar uma seta com as opções. Desative **Lista pendente na célula** para ocultar a seta.
6. Para **Personalizado**, escreva uma fórmula que começa por =. A entrada é permitida quando a fórmula for verdadeira.
7. **Ignorar em branco** permite que uma célula vazia passe.

No separador **Mensagem de Entrada**, marque a caixa e escreva um título e uma mensagem. Estes aparecem junto à célula sempre que ela for selecionada.

No separador **Alerta de Erro**, escolha o que acontece a uma entrada não permitida. **Parar** recusa-a. **Aviso** pergunta se deve manter a entrada. **Informação** informa o utilizador e mantém a entrada. Escreva um título e uma mensagem para o alerta.

**Assinalar Dados Inválidos** (junto a **Validação de Dados**) desenha um círculo vermelho à volta de cada célula da folha cujo valor a sua regra não permite, como um número que já estava lá antes da regra, ou o resultado de uma fórmula. Uma mensagem indica se todas as células estão corretas. Uma célula que corrigir perde o círculo. **Limpar Assinalações de Validação** retira todos os círculos.

**Limpar Tudo** retira as regras das células selecionadas. A nova regra substitui qualquer regra que as células selecionadas tivessem; as outras células mantêm as suas. Alterar regras é um passo que pode anular, e as regras ficam guardadas no ficheiro.

## Consolidar

Use esta opção para combinar intervalos semelhantes, como a mesma tabela em várias folhas, numa única tabela.

1. Escolha **Consolidar** no grupo **Ferramentas de Dados** do separador **Dados**.
2. Escolha a **Função** com que combinar, como **Soma** ou **Média**.
3. Escreva um intervalo na caixa e prima **Adicionar**. Use **Folha2!A1:C10** para um intervalo noutra folha. Adicione cada intervalo que pretende combinar. Selecione um intervalo na lista e prima **Eliminar** para o retirar.
4. Se os intervalos tiverem nomes na primeira linha, marque **A linha superior contém os nomes das colunas**. Se tiverem nomes na primeira coluna, marque **A coluna da esquerda contém os nomes das linhas**.
5. Em **Colocar o resultado em**, escreva a primeira célula do resultado.
6. Marque **Criar ligações aos dados de origem** se cada resultado tiver de ser uma fórmula que lê as células de origem, para que mude quando estas mudarem. Sem esta opção, os resultados são números simples.
7. Prima **OK**.

Sem nomes marcados, combina-se a célula que ocupa o mesmo lugar em cada intervalo (a primeira célula de todos os intervalos, depois a segunda, e assim por diante). Com os nomes marcados, combinam-se as células com os mesmos nomes, independentemente da sua posição em cada intervalo, e um nome que apenas alguns intervalos tenham ainda assim obtém uma linha ou coluna. Consolidar é um passo que pode anular.
