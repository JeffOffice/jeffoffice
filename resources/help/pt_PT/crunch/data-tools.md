# Ferramentas de dados
<!-- keywords: formulário, formulário de dados, introdução de dados, registo, registos, novo registo, localizar seguinte, localizar anterior, critérios, gestor de cenários, cenários, e se, o que acontece se, resumo de cenário, dados, separador dados, texto para colunas, dividir, separador, vírgula, tabulação, largura fixa, remover duplicados, atingir objetivo, subtotal, totais, filtro avançado, condições, validação de dados, assinalar dados inválidos, oval vermelho, limpar assinalações de validação, consolidar, combinar intervalos, permitir, lista, lista pendente, mensagem de entrada, alerta de erro, linhas repetidas, único, ficheiro de texto, csv, a partir de texto, colunas, delimitador, aspas, editar ligações, atualizar valores, alterar origem, ligações, externo, origem, quebrar ligação, verificar estado, abrir origem -->

O separador **Dados** tem as ferramentas que trabalham sobre uma lista de valores: obter texto de um ficheiro, ordenar uma lista, separar texto em colunas e eliminar linhas repetidas.

## Do Texto/CSV

**Do Texto/CSV** abre um ficheiro de texto (.csv, .tsv ou .txt). Faz o mesmo que **Abrir Ficheiro de Texto** no separador Ficheiro. O JeffCrunch pergunta como é que o ficheiro está dividido em colunas e que tipo de texto contém, e depois apresenta-o como um novo livro.

## Editar Ligações

Se um livro tiver fórmulas que leem outro livro (como ='[Preços.xlsx]Folha1'!A1), **Editar Ligações**, no grupo **Obter Dados**, lista esses outros livros, que se chamam origens. Cada linha mostra onde está a origem e o respetivo estado. Até o livro ser aberto novamente com as suas origens, mostra os números que o respetivo ficheiro guardou a partir delas.

- **Verificar Estado** procura cada origem neste computador e indica **Encontrada**, ou indica que não foi encontrada. Uma origem na Web não é obtida, pelo que o estado indica que não foi verificada.
- **Atualizar Valores** lê a origem escolhida deste computador e dá a cada célula que seja apenas uma referência a uma das suas células o valor que essa célula tem agora. Uma célula que faça mais com o valor, como `=[1]Preços!A1*2`, mantém o número que o respetivo ficheiro guardou, e a mensagem por baixo da lista indica quantas células foram atualizadas e quantas não foram. Atualizar Valores é um único passo, pelo que **Anular** repõe os valores antigos. Os novos valores são também escritos na parte da própria ligação dentro do ficheiro ao guardar, para que o livro os mostre da próxima vez que for aberto, mesmo quando a origem não está presente.
- **Alterar Origem** permite escolher outro ficheiro para a ligação escolhida, por exemplo quando o livro foi movido. As fórmulas mantêm-se como estão. O novo local é escrito no ficheiro ao guardar, e **Anular** desfaz a alteração.
- **Abrir Origem** abre a origem escolhida numa nova janela, quando puder ser encontrada.
- **Quebrar Ligação** transforma cada célula que lê a origem escolhida no valor que mostra agora, e um nome que representava um local nela contido em **#REF!**. O JeffCrunch pede confirmação primeiro. Quebrar Ligação é um único passo, pelo que **Anular** repõe as fórmulas, os nomes e as ligações, até guardar o livro. Quando não resta nenhuma ligação, o ficheiro é guardado sem as partes que as continham.

## Formulário

**Formulário** (no grupo **Ferramentas de Dados** e na caixa de pesquisa) mostra uma linha de cada vez de uma lista, como um pequeno formulário. Uma lista é uma tabela, ou um bloco de células com uma linha de nomes no topo e uma linha de dados por baixo de cada nome. Clique primeiro em qualquer célula da lista.

- Cada caixa corresponde a uma coluna, identificada pelo nome da linha superior. Escreva numa caixa para alterar o valor. Uma caixa esbatida contém uma fórmula: o formulário mostra o resultado, e a fórmula é alterada na folha.
- **Localizar Seguinte** e **Localizar Anterior** passam para a linha seguinte e para a anterior. Ao mudar para outra linha, ou ao premir **Enter**, o que escreveu nesta linha é gravado.
- **Novo** mostra um formulário vazio. Preencha-o e, em seguida, prima **Novo** novamente, **Localizar Seguinte** ou **Fechar**. A linha é acrescentada por baixo da última linha da lista. Uma tabela cresce uma linha. Se uma célula por baixo da lista não estiver vazia, o formulário avisa e não acrescenta nada.
- **Eliminar** remove a linha que está a ver, depois de pedir confirmação. As linhas por baixo dela sobem. Uma tabela mantém sempre pelo menos uma linha de dados. Uma linha oculta, ou que um filtro oculta, não é mostrada no formulário, não é contada e nunca é eliminada por ele. Se a lista for demasiado longa para ser percorrida por inteiro, o formulário indica que mostra apenas as primeiras linhas.
- **Repor** desfaz o que escreveu nesta linha, antes de sair dela.
- **Critérios** permite procurar linhas. Escreva uma condição numa ou mais caixas e, em seguida, prima **Localizar Seguinte** ou **Localizar Anterior**. Uma condição é um valor, como **Ana** (o texto corresponde ao início da palavra, e **?** e ***** representam letras), ou um valor precedido de um sinal: **>100**, **<=5**, **<>0** ou **=Lisboa**. Uma linha tem de cumprir todas as condições que escreveu. Prima **Formulário** para voltar atrás, ou **Limpar** para recomeçar. Enquanto existirem critérios definidos, **Localizar Seguinte** e **Localizar Anterior** saltam as linhas que não os cumprem.
- **Fechar** grava a linha e fecha o formulário.

Cada linha que altera, acrescenta ou elimina é um passo, pelo que **Anular** na folha a desfaz de uma só vez.

## Ordenar e filtrar

O grupo **Ordenar e Filtrar** tem os mesmos comandos do menu **Ordenar e Filtrar Linhas** no separador Base. Veja [Ordenar e filtrar](sort-filter).

## Texto para Colunas

Use esta opção quando uma coluna contém vários pedaços de texto que pretende ter em colunas separadas, como "Silva, Ana, 31".

1. Selecione as células de **uma** coluna.
2. Escolha **Texto para Colunas** no separador **Dados**.
3. Escolha como dividir o texto:
   - **Dividir num separador**: marque os sinais que se encontram entre os pedaços: **Tabulação**, **Ponto e vírgula**, **Vírgula**, **Espaço** ou **Outro**, onde escreve o seu próprio sinal. **Tratar separadores consecutivos como um só** evita que dois sinais de vírgula seguidos criem um pedaço vazio. **Texto entre aspas** indica o sinal que envolve um pedaço que não deve ser dividido, como "Lee, Bo". Dentro das aspas, dois sinais seguidos representam um só.
   - **Dividir em posições fixas**: escreva as posições onde começa uma nova coluna, contando os carateres a partir de 1. Por exemplo, 5, 12 começa uma nova coluna no quinto e no décimo segundo carácter.
4. Escolha o que é cada pedaço: **Geral** lê números e datas como se os tivesse escrito, e **Texto** mantém cada pedaço tal como está, pelo que "007" continua a ser "007".
5. Em **Colocar os pedaços a partir de**, escreva a primeira célula para os pedaços. Começa como a primeira célula que selecionou.
6. A caixa por baixo das opções mostra as primeiras linhas divididas da forma escolhida. Prima **OK**.

Os pedaços são colocados nas células à direita da primeira célula, substituindo o que estava nelas. Texto para Colunas é um passo que pode ser anulado.

## Remover Duplicados

Use esta opção para manter apenas a primeira de cada grupo de linhas repetidas.

1. Selecione as células da lista, ou uma célula dentro dela. O JeffCrunch usa todo o bloco de dados à volta de uma única célula.
2. Escolha **Remover Duplicados**.
3. Marque **Os meus dados têm linha de cabeçalho** se a primeira linha tiver nomes. Essa linha mantém-se e não é comparada.
4. Marque as colunas que têm de coincidir para que duas linhas sejam consideradas repetidas. Duas linhas só se repetem quando todas as colunas marcadas são iguais. Letras maiúsculas e minúsculas são consideradas iguais, e uma fórmula conta pelo valor que devolve.
5. Prima **OK**. A caixa de diálogo indica quantas linhas foram removidas e quantas restam.

As linhas por baixo de uma linha removida sobem dentro das células selecionadas, com os respetivos formatos e fórmulas. As células por baixo da seleção não se movem. As linhas vazias que ficam no fim da seleção são limpas. Remover Duplicados é um passo que pode ser anulado.

## Atingir Objetivo

Use esta opção quando conhece o resultado que pretende, mas não o número que o produz. Por exemplo, uma célula de pagamento contém uma fórmula que usa uma taxa de juro, e pretende que o pagamento seja 500. O Atingir Objetivo experimenta números na célula da taxa até que o pagamento seja 500.

1. Escolha **Atingir Objetivo** no separador **Dados**.
2. Em **Célula com a fórmula**, escreva a célula que contém a fórmula, como B3. Começa como a célula que selecionou.
3. Em **Valor pretendido**, escreva o resultado que pretende que a fórmula devolva.
4. Em **Célula a alterar**, escreva a célula que o Atingir Objetivo pode alterar, como B1. Tem de conter um número, não uma fórmula.
5. Prima **OK**.

Se o Atingir Objetivo encontrar um número, este fica na célula a alterar, e a barra de estado indica qual é. Se não o conseguir, nada é alterado e é informado. Experimente outro número inicial na célula a alterar e verifique se a fórmula usa realmente essa célula. Atingir Objetivo é um passo que pode ser anulado.

## Gestor de Cenários

Um cenário é um conjunto com nome de valores para algumas células. Use cenários para experimentar "e se" numa só folha. Por exemplo, guarde um cenário "Vendas baixas" e um cenário "Vendas altas" para as mesmas células de entrada, e mostre o que pretende consultar.

1. Escolha **Gestor de Cenários** no separador **Dados**.
2. Prima **Adicionar**. Escreva um nome, como Vendas altas. Em **Células a alterar**, escreva as células que o cenário define, como B1:B3 ou D5. Um cenário pode alterar até 32 células. As células que selecionou são preenchidas automaticamente. Cada célula começa com o valor que tem agora. Escreva na tabela o valor que este cenário deve usar e prima **OK**.
3. Escolha um cenário na lista e prima **Mostrar** para colocar os respetivos valores nas células. Clicar duas vezes num nome faz o mesmo. Prima **Editar** para alterar o nome, o comentário ou os valores. Prima **Eliminar** para o remover.

Mostrar, Adicionar, Editar e Eliminar são cada um um passo que pode ser anulado.

**Resumo** coloca todos os cenários lado a lado numa nova folha chamada Resumo de Cenário. Pede as células de resultado, que são as células que pretende comparar, como o total em B10. Para cada cenário, o resumo mostra os valores das células a alterar e o resultado das células de resultado, calculados com os valores desse cenário. Uma coluna chamada Valores Atuais mostra como estavam as células quando criou o resumo. As suas próprias células mantêm-se como estavam.

Os cenários ficam guardados no livro. Um livro criado por outro programa mantém os seus cenários, e pode vê-los aqui. Uma cópia guardada como ficheiro .xls antigo ou como .ods não os contém, e ao guardar é indicado esse facto.

## Subtotal

Use esta opção para colocar uma linha de totais por baixo de cada grupo de linhas que têm a mesma entrada numa coluna, e um total geral no fim. A lista precisa de uma linha de cabeçalho e deve estar ordenada pela coluna que agrupa, para que as entradas iguais fiquem juntas.

1. Selecione uma célula da lista.
2. Escolha **Subtotal** no grupo **Estrutura de Tópicos** do separador **Dados**.
3. Em **A cada alteração em**, escolha a coluna que agrupa as linhas, como Região.
4. Em **Utilizar função**, escolha o que calcular: **Soma**, **Contar**, **Média**, **Máx** e **Mín**, entre outras.
5. Em **Adicionar subtotal a**, marque cada coluna a totalizar.
6. Deixe **Substituir subtotais atuais** marcado para substituir os subtotais que já existem. Prima **OK**.

Cada linha de total diz "Total Este" (ou a entrada a que se refere) e contém uma fórmula SUBTOTAL, pelo que ignora as outras linhas de total. As linhas de cada grupo ficam agrupadas, pelo que pode usar os botões 1, 2 e 3 ao lado das linhas para mostrar apenas os totais. Para remover os totais, abra **Subtotal** novamente e prima **Remover Tudo**. Acrescentar ou remover subtotais é um passo que pode ser anulado.

## Filtro Avançado

Os botões de filtro escolhem linhas uma coluna de cada vez. **Avançado** (no grupo **Ordenar e Filtrar**) permite escrever as condições na folha, pelo que podem ter a extensão que for necessária.

1. Junto à lista, copie os cabeçalhos das colunas que pretende testar para uma linha de células vazias.
2. Por baixo de cada cabeçalho, escreva uma condição. As condições na **mesma linha** têm de ser todas verdadeiras (isto é "e"). As condições em **linhas diferentes** são alternativas: uma linha da lista só precisa de cumprir uma dessas linhas (isto é "ou").
3. Escolha **Avançado**. Confirme que **Intervalo da lista** é a lista com a respetiva linha de cabeçalho, e escreva as células dos seus cabeçalhos e condições em **Intervalo das condições**.
4. Escolha **Ocultar as outras linhas, na lista**, ou **Copiar as linhas para outro local** e escreva a primeira célula do local, como H1. Marque **Omitir linhas que repetem uma anterior** para copiar apenas uma de cada.
5. Prima **OK**.

Como escrever uma condição:

- Uma palavra, como **An**, escolhe o texto que começa por essa palavra. Letras maiúsculas e minúsculas são consideradas iguais. **?** representa qualquer carácter e ***** qualquer sequência de carateres.
- Um número escolhe as células que lhe são iguais.
- **=Ana** escolhe o texto que é exatamente Ana. **=** sozinho escolhe as células vazias.
- **>**, **>=**, **<**, **<=** e **<>** (diferente) antes de um número ou de uma palavra fazem a comparação, como **>25**. **<>** sozinho escolhe as células que não estão vazias.

Uma linha de condições sem nada escrito escolhe todas as linhas. Para voltar a mostrar as linhas ocultas, escolha **Limpar**. Uma cópia é feita de imediato e não muda quando a lista muda. Filtro Avançado é um passo que pode ser anulado.

## Validação de Dados

Use esta opção para definir o que pode ser escrito em algumas células, de modo a que uma entrada errada seja impedida ou questionada.

1. Selecione as células.
2. Escolha **Validação de Dados** no grupo **Ferramentas de Dados** do separador **Dados**.
3. No separador **Definições**, escolha o que **Permitir**: **Qualquer valor** (sem regra), **Número inteiro**, **Decimal**, **Lista**, **Data**, **Hora**, **Comprimento do texto** ou **Personalizado**.
4. Para um número, data, hora ou comprimento do texto, escolha **Dados** (**entre**, **não está entre**, **igual a**, e assim por diante) e escreva os limites. Um limite pode ser um valor ou uma fórmula que começa por =.
5. Para uma **Lista**, escreva as opções separadas por vírgulas, como Vermelho,Verde,Azul, ou escreva um intervalo de células depois de =, como =$A$1:$A$5. A célula passa então a mostrar uma seta com as opções. Desative **Lista pendente na célula** para ocultar a seta.
6. Para **Personalizado**, escreva uma fórmula que começa por =. A entrada é permitida quando a fórmula for verdadeira.
7. **Ignorar em branco** permite que uma célula vazia passe.

No separador **Mensagem de Entrada**, marque a caixa e escreva um título e uma mensagem. Estas são apresentadas junto à célula sempre que ela estiver selecionada.

No separador **Alerta de Erro**, escolha o que acontece a uma entrada não permitida. **Parar** recusa-a. **Aviso** pergunta se a deve manter. **Informação** informa a pessoa e mantém a entrada. Escreva um título e uma mensagem para o alerta.

**Assinalar Dados Inválidos** (junto a **Validação de Dados**) coloca um oval vermelho em redor de cada célula da folha cujo valor a sua regra não permite, como um número que já lá estava antes da regra, ou o resultado de uma fórmula. Uma mensagem indica se todas as células estão corretas. Uma célula que corrigir perde o seu oval. **Limpar Assinalações de Validação** remove todos os ovais.

**Limpar Tudo** retira as regras das células selecionadas. A nova regra substitui qualquer regra que as células selecionadas tivessem; as outras células mantêm as suas. Alterar regras é um passo que pode ser anulado, e as regras ficam guardadas no ficheiro.

## Consolidar

Use esta opção para combinar intervalos semelhantes, como a mesma tabela de várias folhas, numa só tabela.

1. Escolha **Consolidar** no grupo **Ferramentas de Dados** do separador **Dados**.
2. Escolha a **Função** com que combinar, como **Soma** ou **Média**.
3. Escreva um intervalo na caixa e prima **Adicionar**. Use **Folha2!A1:C10** para um intervalo noutra folha. Adicione cada intervalo que pretende combinar. Selecione um intervalo na lista e prima **Eliminar** para o retirar.
4. Se os intervalos tiverem nomes na primeira linha, marque **A linha superior contém os nomes das colunas**. Se tiverem nomes na primeira coluna, marque **A coluna da esquerda contém os nomes das linhas**.
5. Em **Colocar o resultado em**, escreva a primeira célula do resultado.
6. Marque **Criar ligações aos dados de origem** se cada resultado deve ser uma fórmula que lê as células de origem, de modo a alterar-se quando elas mudarem. Sem esta opção, os resultados são números simples.
7. Prima **OK**.

Sem nenhum nome marcado, combina-se a célula que ocupa a mesma posição em cada intervalo (a primeira célula de cada intervalo, depois a segunda, e assim por diante). Com os nomes marcados, combinam-se as células com os mesmos nomes, onde quer que estejam em cada intervalo, e um nome que apenas alguns intervalos têm continua a receber uma linha ou coluna. Consolidar é um passo que pode ser anulado.
