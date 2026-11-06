# Ferramentas de Dados
<!-- keywords: gestor de cenários, cenários, e se, o que acontece se, resumo de cenário, dados, separador dados, texto para colunas, dividir, separador, vírgula, tabulação, largura fixa, remover duplicados, atingir objetivo, subtotal, totais, filtro avançado, condições, validação de dados, assinalar dados inválidos, elipse vermelha, limpar assinalações de validação, consolidar, combinar intervalos, permitir, lista, lista pendente, mensagem de entrada, alerta de erro, linhas repetidas, único, ficheiro de texto, csv, de texto, colunas, delimitador, aspas, editar ligações, ligações, externo, origem, interromper ligação, verificar estado, abrir origem -->

O separador **Dados** tem as ferramentas que trabalham sobre uma lista de valores: obter texto de um ficheiro, ordenar uma lista, dividir texto em colunas e eliminar linhas repetidas.

## De Texto/CSV

**De Texto/CSV** abre um ficheiro de texto (.csv, .tsv ou .txt). É o mesmo que **Abrir Ficheiro de Texto** no separador Ficheiro. O JeffCrunch pergunta como o ficheiro está dividido em colunas e que tipo de texto contém, e depois mostra-o como um novo livro.

## Editar Ligações

Se um livro tiver fórmulas que leem outro livro (como ='[Preços.xlsx]Folha1'!A1), **Editar Ligações** no grupo **Obter Dados** lista esses outros livros, que se designam por origens. Cada linha mostra onde está a origem e o respetivo estado. Até o livro ser aberto novamente com as suas origens, mostra os números que o respetivo ficheiro guardou dessas origens.

- **Verificar Estado** procura cada origem neste computador e indica **Encontrada**, ou indica que não foi encontrada. Uma origem na Web não é obtida, pelo que o estado indica que não foi verificada.
- **Abrir Origem** abre a origem escolhida numa nova janela, quando for possível encontrá-la.
- **Interromper Ligação** transforma todas as células que leem a origem escolhida no valor que mostram neste momento, e um nome que representava um local nessa origem em **#REF!**. Esta ação não pode ser anulada, pelo que o JeffCrunch pede confirmação primeiro. Quando não restar nenhuma ligação, o ficheiro é guardado sem as partes que as continham.

## Ordenar e filtrar

O grupo **Ordenar e Filtrar** tem os mesmos comandos do menu **Ordenar e Filtrar Linhas** no separador Base. Consulte [Ordenar e filtrar](sort-filter).

## Texto para Colunas

Use esta opção quando uma coluna tiver vários pedaços de texto que pretende ter em colunas separadas, como "Silva, Ana, 31".

1. Selecione as células de **uma** coluna.
2. Escolha **Texto para Colunas** no separador **Dados**.
3. Escolha como dividir o texto:
   - **Dividir com um separador**: marque os sinais que se situam entre as partes: **Tabulação**, **Ponto e vírgula**, **Vírgula**, **Espaço** ou **Outro**, em que escreve o seu próprio sinal. **Tratar separadores consecutivos como um só** evita que duas vírgulas seguidas criem uma parte vazia. **Texto entre aspas** indica o sinal que envolve uma parte que não deve ser dividida, como "Lee, Bo". Dentro das aspas, dois sinais seguidos valem por um.
   - **Dividir em posições fixas**: escreva as posições em que começa uma nova coluna, contando os carateres a partir de 1. Por exemplo, 5, 12 inicia uma nova coluna no quinto e no décimo segundo carácter.
4. Escolha o que é cada parte: **Geral** lê números e datas como se os tivesse escrito, e **Texto** mantém cada parte tal como está, pelo que "007" se mantém "007".
5. Em **Colocar as partes a partir de**, escreva a primeira célula para as partes. Inicialmente é a primeira célula que selecionou.
6. A caixa abaixo das opções mostra as primeiras linhas divididas da forma escolhida. Prima **OK**.

As partes são colocadas nas células à direita da primeira célula, substituindo o que lá estava. Texto para Colunas é uma ação que pode anular.

## Remover Duplicados

Use esta opção para manter apenas a primeira de cada grupo de linhas repetidas.

1. Selecione as células da lista, ou uma célula dentro dela. O JeffCrunch usa todo o bloco de dados em redor de uma única célula.
2. Escolha **Remover Duplicados**.
3. Marque **Os meus dados têm linha de cabeçalho** se a primeira linha contiver nomes. Essa linha mantém-se e não é comparada.
4. Marque as colunas que têm de coincidir para que duas linhas sejam consideradas repetidas. Duas linhas só se repetem quando todas as colunas marcadas são iguais. Letras maiúsculas e minúsculas são consideradas iguais, e uma fórmula conta pelo valor que produz.
5. Prima **OK**. A caixa de diálogo indica quantas linhas foram removidas e quantas restam.

As linhas abaixo de uma linha removida sobem dentro das células selecionadas, com os respetivos formatos e fórmulas. As células abaixo da seleção não se deslocam. As linhas vazias que ficam no fim da seleção são limpas. Remover Duplicados é uma ação que pode anular.

## Atingir Objetivo

Use esta opção quando conhece a resposta que quer, mas não o número que a produz. Por exemplo, uma célula de pagamento tem uma fórmula que usa uma taxa de juro, e pretende que o pagamento seja 500. O Atingir Objetivo experimenta números na célula da taxa até o pagamento ser 500.

1. Escolha **Atingir Objetivo** no separador **Dados**.
2. Em **Célula com a fórmula**, escreva a célula que contém a fórmula, como B3. Inicialmente é a célula que selecionou.
3. Em **Valor pretendido**, escreva a resposta que pretende que a fórmula dê.
4. Em **Célula a alterar**, escreva a célula que o Atingir Objetivo pode alterar, como B1. Tem de conter um número, não uma fórmula.
5. Prima **OK**.

Se o Atingir Objetivo encontrar um número, este fica na célula a alterar, e a barra de estado indica qual é. Se não conseguir, nada muda e é informado. Experimente outro número inicial na célula a alterar e verifique se a fórmula usa realmente essa célula. Atingir Objetivo é uma ação que pode anular.

## Gestor de Cenários

Um cenário é um conjunto nomeado de valores para algumas células. Use cenários para experimentar "e se" numa única folha. Por exemplo, guarde um cenário "Vendas baixas" e um cenário "Vendas altas" para as mesmas células de entrada, e mostre aquele que pretende analisar.

1. Escolha **Gestor de Cenários** no separador **Dados**.
2. Prima **Adicionar**. Escreva um nome, como Vendas altas. Em **Células a alterar**, escreva as células que o cenário define, como B1:B3 ou D5. Um cenário pode alterar até 32 células. As células que selecionou são preenchidas automaticamente. Cada célula começa com o valor que tem neste momento. Escreva na tabela o valor que este cenário deve usar e prima **OK**.
3. Escolha um cenário na lista e prima **Mostrar** para colocar os respetivos valores nas células. Clicar duas vezes num nome faz o mesmo. Prima **Editar** para alterar o nome, o comentário ou os valores. Prima **Eliminar** para o remover.

Mostrar, Adicionar, Editar e Eliminar são, cada um, uma ação que pode anular.

**Resumo** coloca todos os cenários lado a lado numa nova folha chamada Resumo do Cenário. Pede as células de resultado, que são as células que pretende comparar, como o total em B10. Para cada cenário, o resumo mostra os valores das células a alterar e o que as células de resultado dão, calculado com os valores desse cenário. Uma coluna chamada Valores Atuais mostra como estavam as células quando fez o resumo. As suas próprias células não são alteradas.

Os cenários ficam guardados no livro. Um livro criado por outro programa mantém os seus cenários, e são aqui apresentados. Uma cópia guardada como ficheiro .xls mais antigo ou como .ods não os contém, e ao guardar é indicado esse facto.

## Subtotal

Use esta opção para colocar uma linha de totais abaixo de cada grupo de linhas que têm a mesma entrada numa coluna, e um total geral no fim. A lista precisa de uma linha de cabeçalho e deve estar ordenada pela coluna que agrupa, para que as entradas iguais fiquem juntas.

1. Selecione uma célula da lista.
2. Escolha **Subtotal** no grupo **Estrutura de Tópicos** do separador **Dados**.
3. Em **A cada alteração em**, escolha a coluna que agrupa as linhas, como Região.
4. Em **Utilizar função**, escolha o que calcular: **Soma**, **Contar**, **Média**, **Máx** e **Mín**, entre outras.
5. Em **Adicionar subtotal a**, marque cada coluna a totalizar.
6. Deixe **Substituir subtotais atuais** marcado para trocar os subtotais que já existam. Prima **OK**.

Cada linha de total diz "Total Leste" (ou a entrada a que se refere) e contém uma fórmula SUBTOTAL, para que ignore as outras linhas de total. As linhas de cada grupo ficam agrupadas, pelo que pode usar os botões 1, 2 e 3 ao lado das linhas para mostrar apenas os totais. Para remover os totais, abra novamente **Subtotal** e prima **Remover Tudo**. Adicionar ou remover subtotais é uma ação que pode anular.

## Filtro Avançado

Os botões de filtro escolhem linhas uma coluna de cada vez. **Avançado** (no grupo **Ordenar e Filtrar**) permite escrever as condições na folha, para que sejam tão extensas quanto necessário.

1. Junto à lista, copie os cabeçalhos das colunas que pretende testar para uma linha de células vazias.
2. Por baixo de cada cabeçalho, escreva uma condição. As condições na **mesma linha** têm de ser todas verdadeiras (isto é "e"). As condições em **linhas diferentes** são alternativas: uma linha da lista só precisa de cumprir uma dessas linhas (isto é "ou").
3. Escolha **Avançado**. Verifique se o **Intervalo da lista** é a lista com a respetiva linha de cabeçalho, e escreva as células dos cabeçalhos e das condições em **Intervalo das condições**.
4. Escolha **Ocultar as outras linhas, na lista**, ou **Copiar as linhas para outro local** e escreva a primeira célula do local, como H1. Marque **Omitir linhas que repetem uma anterior** para copiar apenas uma de cada.
5. Prima **OK**.

Como escrever uma condição:

- Uma palavra, como **Ana**, escolhe texto que começa por ela. Letras maiúsculas e minúsculas são consideradas iguais. **?** representa qualquer carácter e ***** qualquer sequência de carateres.
- Um número escolhe células que lhe são iguais.
- **=Ana** escolhe texto que é exatamente Ana. **=** sozinho escolhe células vazias.
- **>**, **>=**, **<**, **<=** e **<>** (diferente) antes de um número ou de uma palavra comparam com ele, como **>25**. **<>** sozinho escolhe células que não estão vazias.

Uma linha de condições sem nada escrito escolhe todas as linhas. Para voltar a mostrar as linhas ocultas, escolha **Limpar**. Uma cópia é feita de imediato e não se altera quando a lista muda. Filtro Avançado é uma ação que pode anular.

## Validação de Dados

Use esta opção para definir o que pode ser escrito em algumas células, para que uma entrada errada seja bloqueada ou questionada.

1. Selecione as células.
2. Escolha **Validação de Dados** no grupo **Ferramentas de Dados** do separador **Dados**.
3. No separador **Definições**, escolha o que **Permitir**: **Qualquer valor** (sem regra), **Número inteiro**, **Decimal**, **Lista**, **Data**, **Hora**, **Comprimento do texto** ou **Personalizado**.
4. Para um número, data, hora ou comprimento de texto, escolha **Dados** (**entre**, **não está entre**, **igual a**, e assim por diante) e escreva os limites. Um limite pode ser um valor ou uma fórmula que começa por =.
5. Para uma **Lista**, escreva as opções separadas por vírgulas, como Vermelho,Verde,Azul, ou escreva um intervalo de células depois de =, como =$A$1:$A$5. A célula passa a mostrar uma seta com as opções. Desative **Lista pendente na célula** para ocultar a seta.
6. Para **Personalizado**, escreva uma fórmula que começa por =. A entrada é permitida quando a fórmula for verdadeira.
7. **Ignorar em branco** permite que uma célula vazia passe.

No separador **Mensagem de Entrada**, marque a caixa e escreva um título e uma mensagem. Estes aparecem junto à célula sempre que ela for selecionada.

No separador **Alerta de Erro**, escolha o que acontece a uma entrada não permitida. **Parar** recusa-a. **Aviso** pergunta se deve mantê-la. **Informação** informa a pessoa e mantém a entrada. Escreva um título e uma mensagem para o alerta.

**Assinalar Dados Inválidos** (junto a **Validação de Dados**) desenha uma elipse vermelha em volta de cada célula da folha cujo valor a sua regra não permite, como um número que já existia antes da regra, ou o resultado de uma fórmula. Uma mensagem indica se todas as células estão corretas. Uma célula que corrija perde a sua elipse. **Limpar Assinalações de Validação** remove todas as elipses.

**Limpar Tudo** retira as regras das células selecionadas. A nova regra substitui qualquer regra que as células selecionadas tivessem; as outras células mantêm as suas. Alterar regras é uma ação que pode anular, e as regras ficam guardadas no ficheiro.

## Consolidar

Use esta opção para combinar intervalos semelhantes, como a mesma tabela em várias folhas, numa só tabela.

1. Escolha **Consolidar** no grupo **Ferramentas de Dados** do separador **Dados**.
2. Escolha a **Função** a combinar, como **Soma** ou **Média**.
3. Escreva um intervalo na caixa e prima **Adicionar**. Use **Folha2!A1:C10** para um intervalo noutra folha. Adicione cada intervalo que pretende combinar. Para retirar um, selecione-o na lista e prima **Eliminar**.
4. Se os intervalos tiverem nomes na primeira linha, marque **A linha superior contém os nomes das colunas**. Se tiverem nomes na primeira coluna, marque **A coluna da esquerda contém os nomes das linhas**.
5. Em **Colocar o resultado em**, escreva a primeira célula do resultado.
6. Marque **Criar ligações aos dados de origem** se cada resultado tiver de ser uma fórmula que lê as suas células de origem, para que mude quando estas mudarem. Sem esta opção, os resultados são números simples.
7. Prima **OK**.

Sem nomes marcados, combina-se a célula na mesma posição em cada intervalo (a primeira célula de todos os intervalos, depois a segunda, e assim por diante). Com nomes marcados, combinam-se as células com os mesmos nomes, estejam onde estiverem em cada intervalo, e um nome que apenas alguns intervalos tenham continua a obter uma linha ou coluna. Consolidar é uma ação que pode anular.
