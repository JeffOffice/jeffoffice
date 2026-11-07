# Ferramentas de Dados
<!-- keywords: gestor de cenários, cenários, e se, o que acontece se, resumo de cenário, dados, separador dados, texto para colunas, dividir, separador, vírgula, tabulação, largura fixa, remover duplicados, atingir objetivo, subtotal, totais, filtro avançado, condições, validação de dados, assinalar dados inválidos, círculo vermelho, limpar assinalações de validação, consolidar, combinar intervalos, permitir, lista, lista pendente, mensagem de entrada, alerta de erro, linhas repetidas, único, ficheiro de texto, csv, a partir de texto, colunas, delimitador, aspas, editar ligações, atualizar valores, alterar origem, ligações, externo, origem, quebrar ligação, verificar estado, abrir origem -->

O separador **Dados** tem as ferramentas que trabalham com uma lista de valores: obter texto de um ficheiro, ordenar uma lista, dividir texto em colunas e eliminar linhas repetidas.

## A partir de Texto/CSV

**A partir de Texto/CSV** abre um ficheiro de texto (.csv, .tsv ou .txt). É o mesmo que **Abrir Ficheiro de Texto** no separador Ficheiro. O JeffCrunch pergunta como o ficheiro está dividido em colunas e que tipo de texto contém, e depois mostra-o como um novo livro.

## Editar Ligações

Se um livro tiver fórmulas que leem outro livro (como ='[Prices.xlsx]Sheet1'!A1), **Editar Ligações** no grupo **Obter Dados** lista esses outros livros, que se chamam origens. Cada linha mostra onde está a origem e o respetivo estado. Até o livro voltar a ser aberto com as suas origens, mostra os números que o respetivo ficheiro guardou a partir delas.

- **Verificar Estado** procura cada origem neste computador e indica **Encontrada**, ou indica que não foi encontrada. Uma origem na Web não é obtida, por isso o estado indica que não foi verificada.
- **Atualizar Valores** lê a origem escolhida deste computador e dá a cada célula que seja apenas uma referência a uma das suas células o valor que essa célula tem agora. Uma célula que faz mais do que usar o valor, como `=[1]Prices!A1*2`, mantém o número que o respetivo ficheiro guardou, e a mensagem por baixo da lista indica quantas células foram atualizadas e quantas não foram. Atualizar Valores é um único passo, por isso **Anular** repõe os valores antigos.
- **Alterar Origem** permite escolher outro ficheiro para a ligação escolhida, por exemplo quando o livro foi movido. As fórmulas mantêm-se como estão. O novo local fica escrito no ficheiro quando guardar, e **Anular** desfaz a alteração.
- **Abrir Origem** abre a origem escolhida numa nova janela, quando for possível encontrá-la.
- **Quebrar Ligação** transforma cada célula que lê a origem escolhida no valor que mostra agora, e um nome que representava um local nessa origem em **#REF!**. Esta ação não pode ser anulada, por isso o JeffCrunch pede confirmação primeiro. Quando não restar nenhuma ligação, o ficheiro é guardado sem as partes que as continham.

## Ordenar e filtrar

O grupo **Ordenar e Filtrar** tem os mesmos comandos do menu **Ordenar e Filtrar Linhas** no separador Base. Veja [Ordenar e filtrar](sort-filter).

## Texto para Colunas

Use isto quando uma coluna tem vários pedaços de texto que pretende ter em colunas separadas, como "Silva, Ana, 31".

1. Selecione as células de **uma** coluna.
2. Escolha **Texto para Colunas** no separador **Dados**.
3. Escolha como dividir o texto:
   - **Dividir num separador**: marque os sinais que existem entre os pedaços: **Tabulação**, **Ponto e vírgula**, **Vírgula**, **Espaço** ou **Outro**, onde escreve o seu próprio sinal. **Tratar separadores consecutivos como um só** evita que duas vírgulas seguidas criem um pedaço vazio. **Texto entre aspas** indica o sinal que envolve um pedaço que não deve ser dividido, como "Lee, Bo". Dentro das aspas, dois sinais seguidos representam um só.
   - **Dividir em posições fixas**: escreva as posições onde começa uma nova coluna, contando os carateres a partir de 1. Por exemplo, 5, 12 começa uma nova coluna no quinto e no décimo segundo carácter.
4. Escolha o que é cada pedaço: **Geral** lê números e datas como se os tivesse escrito, e **Texto** mantém cada pedaço tal como está, para que "007" continue a ser "007".
5. Em **Colocar as partes a partir de**, escreva a primeira célula para as partes. Inicialmente é a primeira célula que selecionou.
6. A caixa por baixo das opções mostra as primeiras linhas divididas da forma escolhida. Prima **OK**.

As partes são colocadas nas células à direita da primeira célula e substituem o que nelas existia. Texto para Colunas é um único passo que pode anular.

## Remover Duplicados

Use isto para manter apenas a primeira de cada grupo de linhas repetidas.

1. Selecione as células da lista, ou uma célula dentro dela. O JeffCrunch usa todo o bloco de dados em volta de uma única célula.
2. Escolha **Remover Duplicados**.
3. Marque **Os meus dados têm linha de cabeçalho** se a primeira linha tiver nomes. Essa linha mantém-se e não é comparada.
4. Marque as colunas que têm de coincidir para que duas linhas sejam consideradas repetidas. Duas linhas só se repetem quando todas as colunas marcadas são iguais. Maiúsculas e minúsculas são consideradas iguais, e uma fórmula conta pelo valor que produz.
5. Prima **OK**. A caixa de diálogo indica quantas linhas foram removidas e quantas ficaram.

As linhas abaixo de uma linha removida sobem dentro das células selecionadas, com os respetivos formatos e fórmulas. As células abaixo da seleção não se deslocam. As linhas vazias que ficam no fim da seleção são limpas. Remover Duplicados é um único passo que pode anular.

## Atingir Objetivo

Use isto quando sabe o resultado que quer, mas não o número que o produz. Por exemplo, uma célula de pagamento tem uma fórmula que usa uma taxa de juro, e pretende que o pagamento seja 500. O Atingir Objetivo experimenta números na célula da taxa até o pagamento ser 500.

1. Escolha **Atingir Objetivo** no separador **Dados**.
2. Em **Célula com a fórmula**, escreva a célula que contém a fórmula, como B3. Inicialmente é a célula que selecionou.
3. Em **Valor pretendido**, escreva o resultado que pretende que a fórmula produza.
4. Em **Célula a alterar**, escreva a célula que o Atingir Objetivo pode alterar, como B1. Tem de conter um número, não uma fórmula.
5. Prima **OK**.

Se o Atingir Objetivo encontrar um número, este fica na célula a alterar, e a barra de estado indica qual é. Se não conseguir, nada muda e é informado. Experimente outro número inicial na célula a alterar e confirme que a fórmula usa realmente essa célula. Atingir Objetivo é um único passo que pode anular.

## Gestor de Cenários

Um cenário é um conjunto nomeado de valores para algumas células. Use cenários para experimentar "e se" numa só folha. Por exemplo, mantenha um cenário "Vendas baixas" e um cenário "Vendas altas" para as mesmas células de entrada, e mostre aquele que quer analisar.

1. Escolha **Gestor de Cenários** no separador **Dados**.
2. Prima **Adicionar**. Escreva um nome, como Vendas altas. Em **Células a alterar**, escreva as células que o cenário define, como B1:B3 ou D5. Um cenário pode alterar até 32 células. As células que selecionou são preenchidas automaticamente. Cada célula começa com o valor que tem agora. Escreva na tabela o valor que este cenário deve usar e prima **OK**.
3. Escolha um cenário na lista e prima **Mostrar** para colocar os respetivos valores nas células. Fazer duplo clique num nome tem o mesmo efeito. Prima **Editar** para alterar o nome, o comentário ou os valores. Prima **Eliminar** para o remover.

Mostrar, Adicionar, Editar e Eliminar são cada um um único passo que pode anular.

**Resumo** coloca todos os cenários lado a lado numa nova folha chamada Resumo de Cenário. Pede as células de resultado, que são as células que pretende comparar, como o total em B10. Para cada cenário, o resumo mostra os valores das células a alterar e o que as células de resultado dão, calculado com os valores desse cenário. Uma coluna chamada Valores Atuais mostra como estavam as células quando criou o resumo. As suas próprias células ficam como estavam.

Os cenários são guardados no livro. Um livro criado noutro programa mantém os seus cenários, e pode vê-los aqui. Uma cópia guardada como ficheiro .xls mais antigo ou como .ods não os contém, e ao guardar é indicado isso.

## Subtotal

Use isto para colocar uma linha de totais abaixo de cada grupo de linhas com a mesma entrada numa coluna, e um total geral no fim. A lista precisa de uma linha de cabeçalho, e deve estar ordenada pela coluna que agrupa, para que as entradas iguais fiquem juntas.

1. Selecione uma célula da lista.
2. Escolha **Subtotal** no grupo **Estrutura de Tópicos** do separador **Dados**.
3. Em **A cada alteração em**, escolha a coluna que agrupa as linhas, como Região.
4. Em **Utilizar função**, escolha o que calcular: **Soma**, **Contar**, **Média**, **Máx.**, **Mín.** e outras.
5. Em **Adicionar subtotal a**, marque cada coluna a totalizar.
6. Deixe **Substituir subtotais atuais** marcado para trocar os subtotais que já existam. Prima **OK**.

Cada linha de total diz "Total Leste" (ou a entrada a que se refere) e contém uma fórmula SUBTOTAL, por isso ignora as outras linhas de total. As linhas de cada grupo ficam agrupadas, para que possa usar os botões 1, 2 e 3 ao lado das linhas e mostrar apenas os totais. Para remover os totais, abra **Subtotal** novamente e prima **Remover Tudo**. Adicionar ou remover subtotais é um único passo que pode anular.

## Filtro Avançado

Os botões de filtro escolhem linhas por uma coluna de cada vez. **Avançado** (no grupo **Ordenar e Filtrar**) permite escrever as condições na folha, para que possam ser tão longas quanto precisar.

1. Junto à lista, copie os cabeçalhos das colunas que pretende testar para uma linha de células vazias.
2. Por baixo de cada cabeçalho, escreva uma condição. As condições na **mesma linha** têm de ser todas verdadeiras (isto é "e"). As condições em **linhas diferentes** são alternativas: uma linha da lista só precisa de cumprir uma dessas linhas (isto é "ou").
3. Escolha **Avançado**. Confirme que o **Intervalo da lista** é a lista com a respetiva linha de cabeçalho, e escreva as células dos seus cabeçalhos e condições em **Intervalo das condições**.
4. Escolha **Ocultar as outras linhas, na lista**, ou **Copiar as linhas para outro local** e escreva a primeira célula do local, como H1. Marque **Omitir linhas que repetem uma anterior** para copiar apenas uma de cada.
5. Prima **OK**.

Como escrever uma condição:

- Uma palavra, como **An**, escolhe texto que começa por essa palavra. Maiúsculas e minúsculas são consideradas iguais. **?** representa qualquer carácter e ***** qualquer sequência de carateres.
- Um número escolhe células iguais a ele.
- **=Ana** escolhe texto que é exatamente Ana. **=** sozinho escolhe células vazias.
- **>**, **>=**, **<**, **<=** e **<>** (diferente) antes de um número ou de uma palavra comparam com ele, como **>25**. **<>** sozinho escolhe células que não estão vazias.

Uma linha de condições sem nada escrito escolhe todas as linhas. Para mostrar novamente as linhas ocultas, escolha **Limpar**. Uma cópia é feita de imediato e não muda quando a lista muda. Filtro Avançado é um único passo que pode anular.

## Validação de Dados

Use isto para definir o que pode ser escrito em algumas células, para que uma entrada errada seja impedida ou questionada.

1. Selecione as células.
2. Escolha **Validação de Dados** no grupo **Ferramentas de Dados** do separador **Dados**.
3. No separador **Definições**, escolha o que **Permitir**: **Qualquer valor** (sem regra), **Número inteiro**, **Decimal**, **Lista**, **Data**, **Hora**, **Comprimento do texto** ou **Personalizado**.
4. Para um número, data, hora ou comprimento do texto, escolha **Dados** (**entre**, **não está entre**, **igual a**, e assim por diante) e escreva os limites. Um limite pode ser um valor ou uma fórmula que começa por =.
5. Para uma **Lista**, escreva as opções separadas por vírgulas, como Vermelho,Verde,Azul, ou escreva um intervalo de células depois de =, como =$A$1:$A$5. A célula passa a mostrar uma seta com as opções. Desative **Lista pendente na célula** para ocultar a seta.
6. Para **Personalizado**, escreva uma fórmula que começa por =. A entrada é permitida quando a fórmula for verdadeira.
7. **Ignorar em branco** deixa passar uma célula vazia.

No separador **Mensagem de Entrada**, marque a caixa e escreva um título e uma mensagem. Estes aparecem junto à célula sempre que ela for selecionada.

No separador **Alerta de Erro**, escolha o que acontece a uma entrada não permitida. **Parar** recusa-a. **Aviso** pergunta se a deve manter. **Informações** informa a pessoa e mantém a entrada. Escreva um título e uma mensagem para o alerta.

**Assinalar Dados Inválidos** (ao lado de **Validação de Dados**) coloca um círculo vermelho em volta de cada célula da folha cujo valor não é permitido pela sua regra, como um número que já lá estava antes da regra, ou o resultado de uma fórmula. Uma mensagem indica se todas as células estão corretas. Uma célula que corrigir perde o seu círculo. **Limpar Assinalações de Validação** retira todos os círculos.

**Limpar Tudo** retira as regras das células selecionadas. A nova regra substitui qualquer regra que as células selecionadas tivessem; as outras células mantêm as suas. Alterar regras é um único passo que pode anular, e as regras ficam guardadas no ficheiro.

## Consolidar

Use isto para combinar intervalos semelhantes, como a mesma tabela de várias folhas, numa só tabela.

1. Escolha **Consolidar** no grupo **Ferramentas de Dados** do separador **Dados**.
2. Escolha a **Função** a usar na combinação, como **Soma** ou **Média**.
3. Escreva um intervalo na caixa e prima **Adicionar**. Use **Folha2!A1:C10** para um intervalo noutra folha. Adicione cada intervalo que pretende combinar. Selecione um intervalo na lista e prima **Eliminar** para o retirar.
4. Se os intervalos tiverem nomes na primeira linha, marque **A linha superior contém os nomes das colunas**. Se tiverem nomes na primeira coluna, marque **A coluna da esquerda contém os nomes das linhas**.
5. Em **Colocar o resultado em**, escreva a primeira célula do resultado.
6. Marque **Criar ligações aos dados de origem** se cada resultado tiver de ser uma fórmula que lê as células de origem, para que mude quando estas mudarem. Sem esta opção, os resultados são números simples.
7. Prima **OK**.

Com nenhum nome marcado, combina-se a célula no mesmo lugar em cada intervalo (a primeira célula de cada intervalo, depois a segunda, e assim por diante). Com os nomes marcados, combinam-se as células com os mesmos nomes, onde quer que estejam em cada intervalo, e um nome que apenas alguns intervalos tenham ainda assim recebe uma linha ou coluna. Consolidar é um único passo que pode anular.
