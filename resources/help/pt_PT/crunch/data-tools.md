# Ferramentas de dados
<!-- keywords: previsão em folha, previsão, valores futuros, intervalo de confiança, sazonalidade, tendência, suavização, formulário, formulário de dados, introdução de dados, registo, registos, novo registo, localizar seguinte, localizar anterior, critérios, gestor de cenários, cenários, e se, o que acontece se, resumo de cenário, dados, separador dados, texto para colunas, dividir, separador, vírgula, tabulação, largura fixa, remover duplicados, atingir objetivo, subtotal, totais, filtro avançado, condições, validação de dados, assinalar dados inválidos, círculo vermelho, limpar círculos de validação, consolidar, combinar intervalos, permitir, lista, lista pendente, mensagem de entrada, alerta de erro, linhas repetidas, único, ficheiro de texto, csv, a partir de texto, colunas, delimitador, aspas, editar ligações, atualizar valores, alterar origem, ligações, externo, origem, quebrar ligação, verificar estado, abrir origem -->

O separador **Dados** tem as ferramentas que trabalham sobre uma lista de valores: obter texto de um ficheiro, ordenar uma lista, dividir texto em colunas e eliminar linhas repetidas.

## A partir de Texto/CSV

**A partir de Texto/CSV** abre um ficheiro de texto (.csv, .tsv ou .txt). Faz o mesmo que **Abrir Ficheiro de Texto** no separador Ficheiro. O JeffCrunch pergunta como é que o ficheiro está dividido em colunas e que tipo de texto contém, e depois mostra-o como um novo livro.

## Editar Ligações

Se um livro tiver fórmulas que leem outro livro (como ='[Preços.xlsx]Folha1'!A1), **Editar Ligações**, no grupo **Obter Dados**, lista esses outros livros, que se designam por origens. Cada linha mostra onde está a origem e o respetivo estado. Até o livro ser aberto novamente com as suas origens, mostra os números que o seu ficheiro guardou a partir delas.

- **Verificar Estado** procura cada origem neste computador e indica **Encontrada**, ou indica que não foi encontrada. Uma origem na Web não é obtida, por isso o estado indica que não foi verificada.
- **Atualizar Valores** lê a origem escolhida a partir deste computador e dá a cada célula que seja apenas uma referência a uma das suas células o valor que essa célula tem agora. Uma célula que faça mais com o valor, como `=[1]Preços!A1*2`, mantém o número que o seu ficheiro guardou, e a mensagem por baixo da lista indica quantas células foram atualizadas e quantas não foram. Atualizar Valores é um só passo, por isso **Anular** repõe os valores antigos. Os novos valores são também escritos na parte da própria ligação no ficheiro ao guardar, para que o livro os mostre da próxima vez que for aberto, mesmo que a origem não esteja presente.
- **Alterar Origem** permite escolher outro ficheiro para a ligação selecionada, por exemplo quando o livro foi movido. As fórmulas mantêm-se como estão. O novo local é escrito no ficheiro ao guardar, e **Anular** desfaz a alteração.
- **Abrir Origem** abre a origem escolhida numa nova janela, quando pode ser encontrada.
- **Quebrar Ligação** transforma cada célula que lê a origem escolhida no valor que mostra neste momento, e um nome que representava um local nessa origem em **#REF!**. O JeffCrunch pede confirmação antes. Quebrar Ligação é um só passo, por isso **Anular** repõe as fórmulas, os nomes e as ligações, até guardar o livro. Quando não resta nenhuma ligação, o ficheiro é guardado sem as partes que as continham.

## Formulário

**Formulário** (no grupo **Ferramentas de Dados** e na caixa de pesquisa) mostra uma linha de cada vez de uma lista, como um pequeno formulário. Uma lista é uma tabela, ou um bloco de células com uma linha de nomes no topo e uma linha de dados por baixo de cada nome. Clique primeiro em qualquer célula da lista.

- Cada caixa corresponde a uma coluna, com a etiqueta do nome da linha superior. Escreva numa caixa para alterar o valor. Uma caixa a cinzento tem uma fórmula: o formulário mostra o resultado, e a fórmula é alterada na folha.
- **Localizar Seguinte** e **Localizar Anterior** avançam para a linha seguinte e para a anterior. Mudar para outra linha, ou premir **Enter**, grava o que escreveu nesta linha.
- **Novo** mostra um formulário vazio. Preencha-o e, depois, prima **Novo** novamente, **Localizar Seguinte** ou **Fechar**. A linha é acrescentada por baixo da última linha da lista. Uma tabela cresce uma linha. Se uma célula por baixo da lista não estiver vazia, o formulário avisa e não acrescenta nada.
- **Eliminar** remove a linha que está a ver, depois de pedir confirmação. As linhas por baixo sobem. Uma tabela mantém sempre pelo menos uma linha de dados. Uma linha oculta, ou que um filtro oculte, não é mostrada no formulário, não é contada e nunca é eliminada por ele. Se a lista for demasiado longa para ser percorrida por completo, o formulário indica que mostra apenas as primeiras linhas.
- **Restaurar** desfaz o que escreveu nesta linha, antes de a deixar.
- **Critérios** permite procurar linhas. Escreva uma condição numa ou mais caixas e, depois, prima **Localizar Seguinte** ou **Localizar Anterior**. Uma condição é um valor, como **Ana** (o texto corresponde ao início da palavra, e **?** e ***** representam letras), ou um valor precedido de um sinal: **>100**, **<=5**, **<>0**, **=Oslo**. Uma linha tem de cumprir todas as condições que escrever. Prima **Formulário** para voltar, ou **Limpar** para recomeçar. Enquanto existirem critérios definidos, **Localizar Seguinte** e **Localizar Anterior** saltam as linhas que não os cumprem.
- **Fechar** grava a linha e fecha o formulário.

Cada linha que alterar, acrescentar ou eliminar é um passo, por isso **Anular** na folha desfaz tudo de uma vez.

## Ordenar e filtrar

O grupo **Ordenar e Filtrar** tem os mesmos comandos do menu **Ordenar e Filtrar Linhas** no separador Base. Veja [Ordenar e filtrar](sort-filter).

## Texto para Colunas

Use esta opção quando uma coluna contém vários textos que pretende ter em colunas separadas, como "Silva, Ana, 31".

1. Selecione as células de **uma** coluna.
2. Escolha **Texto para Colunas** no separador **Dados**.
3. Escolha como dividir o texto:
   - **Dividir num separador**: marque os sinais que existem entre as partes: **Tabulação**, **Ponto e vírgula**, **Vírgula**, **Espaço** ou **Outro**, onde escreve o seu próprio sinal. **Tratar separadores consecutivos como um só** evita que duas vírgulas seguidas criem uma parte vazia. **Texto entre aspas** indica o sinal que envolve uma parte que não deve ser dividida, como "Lee, Bo". Dentro das aspas, dois sinais seguidos representam um só.
   - **Dividir em posições fixas**: escreva as posições onde começa uma nova coluna, contando os carateres a partir de 1. Por exemplo, 5, 12 inicia uma nova coluna no quinto e no décimo segundo carácter.
4. Escolha o tipo de cada parte: **Geral** lê os números e as datas como se os tivesse escrito, e **Texto** mantém cada parte tal como está, para que "007" continue a ser "007".
5. Em **Colocar as partes a partir de**, escreva a primeira célula para as partes. Inicialmente, é a primeira célula que selecionou.
6. A caixa por baixo das opções mostra as primeiras linhas divididas da forma escolhida. Prima **OK**.

As partes são colocadas nas células à direita da primeira célula e substituem o que nelas estava. Texto para Colunas é um passo que pode anular.

## Remover Duplicados

Use esta opção para manter apenas a primeira de cada grupo de linhas repetidas.

1. Selecione as células da lista, ou uma célula dentro dela. O JeffCrunch considera todo o bloco de dados à volta de uma única célula.
2. Escolha **Remover Duplicados**.
3. Marque **Os meus dados têm linha de cabeçalho** se a primeira linha contiver nomes. Essa linha mantém-se e não é comparada.
4. Marque as colunas que têm de coincidir para que duas linhas sejam consideradas repetidas. Duas linhas só se repetem quando todas as colunas marcadas são iguais. Maiúsculas e minúsculas são consideradas iguais, e uma fórmula conta pelo valor que devolve.
5. Prima **OK**. A caixa de diálogo indica quantas linhas foram removidas e quantas restam.

As linhas por baixo de uma linha removida sobem dentro das células selecionadas, com os respetivos formatos e fórmulas. As células por baixo da seleção não se deslocam. As linhas vazias que ficam no fim da seleção são limpas. Remover Duplicados é um passo que pode anular.

## Atingir Objetivo

Use esta opção quando sabe a resposta que pretende, mas não o número que a produz. Por exemplo, uma célula de pagamento tem uma fórmula que usa uma taxa de juro, e pretende que o pagamento seja 500. O Atingir Objetivo experimenta números na célula da taxa até que o pagamento seja 500.

1. Escolha **Atingir Objetivo** no separador **Dados**.
2. Em **Célula com a fórmula**, escreva a célula que contém a fórmula, como B3. Inicialmente, é a célula que selecionou.
3. Em **Valor pretendido**, escreva a resposta que pretende que a fórmula dê.
4. Em **Célula a alterar**, escreva a célula que o Atingir Objetivo pode alterar, como B1. Tem de conter um número, e não uma fórmula.
5. Prima **OK**.

Se o Atingir Objetivo encontrar um número, este fica na célula a alterar, e a barra de estado indica qual é. Se não conseguir, nada muda e é informado disso. Experimente um número inicial diferente na célula a alterar e confirme que a fórmula usa mesmo essa célula. Atingir Objetivo é um passo que pode anular.

## Gestor de Cenários

Um cenário é um conjunto nomeado de valores para algumas células. Use cenários para experimentar "e se" numa só folha. Por exemplo, mantenha um cenário "Vendas baixas" e um cenário "Vendas altas" para as mesmas células de entrada, e mostre o que pretende analisar.

1. Escolha **Gestor de Cenários** no separador **Dados**.
2. Prima **Adicionar**. Escreva um nome, como Vendas altas. Em **Células a alterar**, escreva as células que o cenário define, como B1:B3 ou D5. Um cenário pode alterar até 32 células. As células que selecionou são preenchidas automaticamente. Cada célula começa com o valor que tem neste momento. Escreva na tabela o valor que este cenário deve usar e prima **OK**.
3. Escolha um cenário na lista e prima **Mostrar** para colocar os seus valores nas células. Clicar duas vezes num nome faz o mesmo. Prima **Editar** para alterar o nome, o comentário ou os valores. Prima **Eliminar** para o remover.

Mostrar, Adicionar, Editar e Eliminar são cada um um passo que pode anular.

**Resumo** coloca todos os cenários lado a lado numa nova folha chamada Resumo de Cenário. Pede as células de resultado, que são as células que pretende comparar, como o total em B10. Para cada cenário, o resumo mostra os valores das células alteradas e o resultado das células de resultado, calculado com os valores desse cenário. Uma coluna chamada Valores Atuais mostra como estavam as células quando criou o resumo. As suas próprias células mantêm-se como estavam.

Os cenários são guardados no livro. Um livro criado por outro programa mantém os seus cenários, e aqui pode vê-los. Uma cópia guardada como ficheiro .xls mais antigo ou como .ods não os contém, e ao guardar é indicado isso.

## Subtotal

Use esta opção para colocar uma linha de totais por baixo de cada grupo de linhas que têm a mesma entrada numa coluna, e um total geral no fim. A lista precisa de uma linha de cabeçalho e deve estar ordenada pela coluna pela qual agrupa, para que as entradas iguais fiquem juntas.

1. Selecione uma célula da lista.
2. Escolha **Subtotal** no grupo **Estrutura de Tópicos** do separador **Dados**.
3. Em **A cada alteração em**, escolha a coluna que agrupa as linhas, como Região.
4. Em **Utilizar função**, escolha o que calcular: **Soma**, **Contagem**, **Média**, **Máx** e **Mín**, entre outras.
5. Em **Adicionar subtotal a**, marque cada coluna a totalizar.
6. Deixe **Substituir subtotais atuais** marcado para substituir subtotais que já existam. Prima **OK**.

Cada linha de total diz "Total Este" (ou a entrada a que corresponde) e contém uma fórmula SUBTOTAL, por isso ignora as outras linhas de total. As linhas de cada grupo ficam agrupadas, pelo que pode usar os botões 1, 2 e 3 ao lado das linhas para mostrar apenas os totais. Para remover os totais, abra **Subtotal** novamente e prima **Remover Tudo**. Acrescentar ou remover subtotais é um passo que pode anular.

## Filtro Avançado

Os botões de filtro escolhem linhas por uma coluna de cada vez. **Avançado** (no grupo **Ordenar e Filtrar**) permite escrever as condições na folha, pelo que podem ser tão longas quanto necessário.

1. Junto à lista, copie os cabeçalhos das colunas que pretende testar para uma linha de células vazias.
2. Por baixo de cada cabeçalho, escreva uma condição. As condições na **mesma linha** têm de ser todas verdadeiras (é o "e"). As condições em **linhas diferentes** são alternativas: uma linha da lista tem de cumprir apenas uma dessas linhas (é o "ou").
3. Escolha **Avançado**. Confirme que o **Intervalo da lista** é a lista com a respetiva linha de cabeçalho, e escreva as células dos seus cabeçalhos e condições em **Intervalo das condições**.
4. Escolha **Ocultar as outras linhas, na lista**, ou **Copiar as linhas para outro local** e escreva a primeira célula do local, como H1. Marque **Omitir linhas que repetem uma anterior** para copiar apenas uma de cada.
5. Prima **OK**.

Como escrever uma condição:

- Uma palavra, como **An**, escolhe o texto que começa por essa palavra. Maiúsculas e minúsculas são consideradas iguais. **?** representa qualquer carácter e ***** qualquer sequência de carateres.
- Um número escolhe as células que lhe são iguais.
- **=Ana** escolhe o texto que é exatamente Ana. **=** sozinho escolhe as células vazias.
- **>**, **>=**, **<**, **<=** e **<>** (diferente) antes de um número ou de uma palavra comparam com ele, como **>25**. **<>** sozinho escolhe as células que não estão vazias.

Uma linha de condições sem nada escrito escolhe todas as linhas. Para voltar a mostrar as linhas ocultas, escolha **Limpar**. Uma cópia é feita de imediato e não se altera quando a lista muda. Filtro Avançado é um passo que pode anular.

## Validação de Dados

Use esta opção para definir o que pode ser escrito em algumas células, para que uma entrada errada seja impedida ou questionada.

1. Selecione as células.
2. Escolha **Validação de Dados** no grupo **Ferramentas de Dados** do separador **Dados**.
3. No separador **Definições**, escolha o que **Permitir**: **Qualquer valor** (sem regra), **Número inteiro**, **Decimal**, **Lista**, **Data**, **Hora**, **Comprimento do texto** ou **Personalizado**.
4. Para um número, uma data, uma hora ou um comprimento de texto, escolha **Dados** (**entre**, **não está entre**, **igual a**, e assim por diante) e escreva os limites. Um limite pode ser um valor ou uma fórmula que começa por =.
5. Para uma **Lista**, escreva as opções separadas por vírgulas, como Vermelho,Verde,Azul, ou escreva um intervalo de células depois de =, como =$A$1:$A$5. A célula passa então a mostrar uma seta com as opções. Desative **Lista pendente na célula** para ocultar a seta.
6. Para **Personalizado**, escreva uma fórmula que começa por =. A entrada é permitida quando a fórmula for verdadeira.
7. **Ignorar em branco** permite que uma célula vazia passe.

No separador **Mensagem de Entrada**, marque a caixa e escreva um título e uma mensagem. Estes aparecem junto à célula sempre que ela for selecionada.

No separador **Alerta de Erro**, escolha o que acontece a uma entrada não permitida. **Parar** recusa-a. **Aviso** pergunta se deve mantê-la. **Informação** avisa a pessoa e mantém a entrada. Escreva um título e uma mensagem para o alerta.

**Assinalar Dados Inválidos** (junto a **Validação de Dados**) coloca um círculo vermelho à volta de cada célula da folha cujo valor não é permitido pela sua regra, como um número que já existia antes da regra, ou o resultado de uma fórmula. Uma mensagem indica se todas as células estão corretas. Uma célula que corrija perde o seu círculo. **Limpar Assinalações de Validação** remove todos os círculos.

**Limpar Tudo** retira as regras das células selecionadas. A nova regra substitui qualquer regra que as células selecionadas tivessem; as outras células mantêm as suas. Alterar regras é um passo que pode anular, e as regras são guardadas no ficheiro.

## Consolidar

Use esta opção para combinar intervalos semelhantes, como a mesma tabela em várias folhas, numa só tabela.

1. Escolha **Consolidar** no grupo **Ferramentas de Dados** do separador **Dados**.
2. Escolha a **Função** com que combinar, como **Soma** ou **Média**.
3. Escreva um intervalo na caixa e prima **Adicionar**. Use **Folha2!A1:C10** para um intervalo noutra folha. Adicione cada intervalo que pretende combinar. Selecione um intervalo na lista e prima **Eliminar** para o retirar.
4. Se os intervalos tiverem nomes na primeira linha, marque **A linha superior contém os nomes das colunas**. Se tiverem nomes na primeira coluna, marque **A coluna da esquerda contém os nomes das linhas**.
5. Em **Colocar o resultado em**, escreva a primeira célula do resultado.
6. Marque **Criar ligações aos dados de origem** se cada resultado tiver de ser uma fórmula que lê as suas células de origem, para mudar quando elas mudarem. Sem esta opção, os resultados são números simples.
7. Prima **OK**.

Sem nomes marcados, combina-se a célula na mesma posição de cada intervalo (a primeira célula de cada intervalo, depois a segunda, e assim por diante). Com os nomes marcados, combinam-se as células com os mesmos nomes, esteja onde estiverem em cada intervalo, e um nome que só alguns intervalos tenham ainda assim recebe uma linha ou coluna. Consolidar é um passo que pode anular.

## Folha de Previsão

A Folha de Previsão cria uma nova folha com uma tabela e um gráfico de valores futuros, calculados a partir de uma linha temporal e de uma série de valores. Encontra a tendência e a sazonalidade que se repete nos valores, e acrescenta uma faixa dentro da qual os valores futuros provavelmente se manterão.

1. Selecione a linha temporal e os valores, duas colunas lado a lado (a linha temporal primeiro). A linha temporal são datas ou números com a mesma distância entre si, como todos os meses ou de 5 em 5 unidades. Uma primeira linha de nomes fica de fora.
2. Escolha **Folha de Previsão** no grupo **Previsão** do separador **Dados**.
3. Uma imagem mostra a previsão e altera-se à medida que altera as opções. Escolha **Gráfico de linhas** ou **Gráfico de colunas**.
4. Defina as opções:
   - **Intervalo da linha temporal** e **Intervalo de valores** são as células a ler. Escreva aqui outras células se a seleção estiver errada.
   - **Início da previsão** fica vazio para começar depois do último ponto. Uma data ou número anterior ao último ponto começa aí e usa apenas os pontos anteriores, para que possa comparar a previsão com o que aconteceu.
   - **Fim da previsão** é o último momento a prever. São previstos, no máximo, 10 000 pontos.
   - **Intervalo de confiança** indica a certeza da faixa, inicialmente 95%.
   - **Sazonalidade** é encontrada ao analisar os valores, ou pode definir o comprimento da época em pontos, ou escolher nenhuma. Os valores devem conter duas épocas completas.
   - **Preencher pontos em falta com** indica o que recebe um momento sem valor: uma linha reta entre os seus vizinhos, ou zero.
   - **Agregar duplicados utilizando** indica como são combinados os valores no mesmo momento: Média, Contagem, CountA, Máx, Mediana, Mín ou Soma.
   - **Incluir estatísticas de previsão** acrescenta uma tabela com Alfa, Beta, Gama, MASE, SMAPE, MAE e RMSE, que indicam o ajuste da previsão aos valores.
5. Se algo estiver errado, a caixa de diálogo indica o quê em linguagem simples (passos que não são iguais, pontos insuficientes, texto nos valores), e **Criar** fica indisponível.
6. Prima **Criar**.

A nova folha é colocada antes da folha em que estava. A sua tabela tem as colunas Linha temporal, Valores, Previsão, Limite inferior de confiança e Limite superior de confiança. A Previsão e os limites são fórmulas que usam `FORECAST.ETS` e `FORECAST.ETS.CONFINT` sobre as colunas Linha temporal e Valores da própria tabela, por isso são recalculadas quando altera um valor. O gráfico mostra os valores, a previsão e a faixa. Folha de Previsão é um passo que pode anular.

Os números podem diferir em alguns por cento dos de outros programas de folha de cálculo, que usam o seu próprio método para ajustar a suavização, e uma previsão que um ficheiro já contenha mantém os valores que o ficheiro guardou, até os dados que ela lê serem alterados.

Um ficheiro com mais de 1 000 000 de pontos não é utilizado.
