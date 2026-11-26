# Data tools
<!-- keywords: previsão, folha de previsão, valores futuros, intervalo de confiança, sazonalidade, tendência, suavização, formulário, formulário de dados, introdução de dados, registo, registos, novo registo, localizar seguinte, localizar anterior, critérios, gestor de cenários, cenários, e se, o que acontece se, resumo de cenário, dados, separador dados, texto para colunas, dividir, separador, vírgula, tabulação, largura fixa, remover duplicados, atingir objetivo, subtotal, totais, filtro avançado, condições, validação de dados, assinalar dados inválidos, círculo vermelho, limpar círculos de validação, consolidar, combinar intervalos, permitir, lista, lista pendente, mensagem de entrada, alerta de erro, linhas repetidas, único, ficheiro de texto, csv, de texto, colunas, delimitador, aspas, editar ligações, atualizar valores, alterar origem, ligações, externo, origem, quebrar ligação, verificar estado, abrir origem -->

O separador **Dados** tem as ferramentas que trabalham sobre uma lista de valores: obter texto de um ficheiro, ordenar uma lista, dividir texto em colunas e eliminar linhas repetidas.

## De Texto/CSV

**De Texto/CSV** abre um ficheiro de texto (.csv, .tsv ou .txt). É o mesmo que **Abrir Ficheiro de Texto** no separador Ficheiro. O JeffCrunch pergunta como o ficheiro está dividido em colunas e o que contém cada coluna, e depois mostra-o como um novo livro. A mesma janela abre quando abre um ficheiro de texto ou utiliza **Colar Importação de Texto**.

A janela adivinha o que contém cada coluna a partir das primeiras 200 linhas do ficheiro e mostra a adivinha na caixa acima da coluna. Altere qualquer coluna antes de abrir o ficheiro. Um tipo que altere nunca é substituído por uma adivinha posterior, mesmo que depois altere o separador. Uma adivinha faz apenas três coisas a uma coluna: deixa a coluna **Como escrito**, que é a forma como um ficheiro é lido quando nada é adivinhado; torna a coluna **Texto** para a proteger; ou torna as datas da coluna numa ordem que os valores comprovam. Estas são as regras:

- Uma coluna fica **Como escrito** quando Como escrito lê corretamente os seus valores: números, verdadeiro e falso, horas, datas escritas com nomes de meses, números com sinal como -5 e +3, palavras e números de versão como 1.13.2. Alterá-los só os estragaria.
- Uma coluna é de **datas** apenas quando todos os valores têm um ano de quatro algarismos (ou são uma data completa como 2024-05-13) e são lidos como data numa só ordem e em nenhuma outra (mês, dia, ano; dia, mês, ano; ou ano, mês, dia). As datas são mostradas como as datas que escreve são mostradas. Quando mais do que uma ordem serve todos os valores, como 05/06/2024, a coluna fica como **Texto**, a janela indica-a e escolhe a ordem na sua caixa.
- Valores com três números e sem ano de quatro algarismos, como 1.13.2 ou 13.05.24, nunca são transformados em datas pela adivinha. Se Como escrito os manteria como texto, a coluna fica Como escrito; se Como escrito os transformaria em datas, a coluna fica **Texto**. Em qualquer dos casos, a janela indica uma coluna que poderia ser de datas com anos de dois algarismos, para que possa escolher a ordem.
- Uma coluna fica **Texto** quando um valor tem um zero à frente de mais algarismos, quando tem mais de 15 algarismos, quando um sinal de mais surge antes de sete ou mais algarismos (um número de telefone), ou quando um expoente como 1E5 aparece junto a outros valores.
- Um valor que começa com um sinal de igual fica a cargo da caixa **Ler fórmulas**. Quando a caixa está marcada, a coluna fica Como escrito para que o valor seja lido como fórmula. Quando não está marcada, a coluna fica **Texto**.
- Marque **A primeira linha tem nomes de colunas** e a primeira linha não é usada na adivinha. A janela marca-a quando a primeira linha é texto sobre números ou datas.
- As colunas depois das primeiras 100 são importadas como texto.

Só as primeiras 200 linhas são analisadas, pelo que um valor mais abaixo pode ser uma surpresa. Numa coluna de datas, um valor mais abaixo que não seja uma data na ordem escolhida é importado como texto, nunca como uma data errada. Numa coluna de Texto, cada valor é importado tal como está escrito. Numa coluna Como escrito, um valor mais abaixo é lido da forma como Como escrito o lê em qualquer outro sítio.

**Colar Importação de Texto** introduz o texto como se o tivesse escrito, com as suas próprias marcas decimais e de milhares, pelo que a sua janela não tem caixas para essas marcas, e a adivinha lê o texto da mesma forma que a colagem. A janela de um ficheiro tem-nas.

## De Ficheiro

**De Ficheiro**, no grupo **Obter Dados**, carrega as linhas de um ficheiro JSON, de um ficheiro XML ou de outro livro como uma tabela. Só pode ser carregado um ficheiro deste computador. Um endereço Web ou um ficheiro de outro computador é recusado.

1. Clique em **De Ficheiro**, depois em **Procurar** e escolha o ficheiro. As primeiras linhas são mostradas de imediato, com o tipo que o JeffCrunch encontrou para cada coluna a partir dessas linhas: número, data, verdadeiro ou falso, ou texto. O ficheiro inteiro é verificado ao ser carregado, pelo que uma coluna pode acabar como texto. Nada na folha muda até clicar em **Carregar**.
2. Quando o ficheiro tem mais do que uma lista, escolha a que carregar. Num ficheiro JSON é uma lista de itens com valores com nome, como `$.items`. Num ficheiro XML é um item que se repete, como `/feed/entry`. Num livro é uma folha, uma tabela ou um intervalo com nome.
3. Um item JSON dentro de um item torna-se uma coluna com o nome `externo.interno`, até quatro níveis. Uma lista de valores simples é juntada numa só célula (os primeiros 10 000 valores), ou distribuída por colunas (as primeiras 16), conforme escolher. Em XML, os atributos e os itens filhos tornam-se colunas. Dois valores diferentes nunca partilham uma coluna: quando dois ficariam com o mesmo nome, o segundo recebe um número acrescentado ou, em XML, o seu prefixo, e uma nota diz isso. Um nome repetido dentro de um item JSON mantém o seu último valor, e uma nota diz isso. Um número demasiado grande ou demasiado pequeno para uma célula é mantido como texto, tal como está escrito, e um carácter que não pode ser lido é mostrado como um sinal de substituição; ambos são indicados em nota.
4. Marque **Carregar todas as colunas como texto** para manter cada valor tal como está escrito. Uma coluna de números com um zero à frente, como 007, é sempre mantida como texto.
5. Escreva a célula onde fica a tabela e clique em **Carregar**. Se a tabela cobrir células que contenham algo, o JeffCrunch pergunta antes de as substituir. Se não houver espaço, ou se a folha estiver protegida, é avisado e nada muda.

A tabela é um passo de **Anular**. Guarda o ficheiro e as suas escolhas. Para ler o ficheiro novamente, clique numa célula da tabela e clique em **Atualizar** no separador Dados. **Atualizar Tudo** é o botão grande ao lado: lê todas as tabelas criadas com Obter Dados e depois atualiza todas as tabelas dinâmicas, tudo como um passo de Anular. A tabela cresce ou diminui conforme as linhas do ficheiro. As colunas que acrescentou ao lado da tabela mantêm-se: uma fórmula que copiou para baixo é preenchida nas novas linhas, e as linhas que deixaram de existir no ficheiro saem também. Se o ficheiro já não existir ou não puder ser lido, é avisado e nada muda. Se as colunas próprias do ficheiro mudaram, é indicado quais estão em falta ou são novas, e nada muda. Crie a tabela novamente com Obter Dados, ou volte a pôr as colunas no ficheiro.

A ligação ao ficheiro é guardada na tabela, num livro `.xlsx` guardado, numa parte que outros programas ignoram. A ligação contém o caminho completo do ficheiro neste computador, pelo que quem receber o livro pode ver esse caminho. Para a retirar antes de partilhar o livro, use **Inspecionar**, depois **Ligações a ficheiros de dados** e depois **Remover**. Uma cópia guardada com outro tipo de ficheiro não a mantém, e **Verificar Compatibilidade** indica-o. **Inspecionar** lista as ligações em **Ligações a ficheiros de dados** e pode removê-las. As tabelas e as suas células mantêm-se.

Os ficheiros são lidos com limites: um ficheiro até 64 MB, 200 000 linhas, 256 colunas e 32 767 carateres numa célula. XML com uma declaração de tipo de documento ou entidades é recusado. Tudo o que seja cortado por um limite é indicado.

## Editar Ligações

Se um livro tiver fórmulas que leem outro livro (como ='[Preços.xlsx]Folha1'!A1), **Editar Ligações**, no grupo **Obter Dados**, lista esses outros livros, que se chamam origens. Cada linha mostra onde está a origem e o seu estado. Até o livro ser aberto novamente com as suas origens, mostra os números que o seu ficheiro guardou a partir delas.

- **Verificar Estado** procura cada origem neste computador e indica **Encontrada**, ou indica que não foi encontrada. Uma origem na Web não é obtida, pelo que o seu estado indica que não foi verificada.
- **Atualizar Valores** lê a origem escolhida deste computador e dá a cada célula que seja apenas uma referência a uma das suas células o valor que essa célula tem agora. Uma célula que faça mais com o valor, como `=[1]Preços!A1*2`, mantém o número que o seu ficheiro guardou, e a mensagem por baixo da lista indica quantas células foram atualizadas e quantas não foram. **Atualizar Valores** é um passo, pelo que **Anular** repõe os valores antigos. Os novos valores são também escritos na parte da ligação dentro do ficheiro quando guarda, para que o livro os mostre na próxima abertura, mesmo que a origem não esteja presente.
- **Alterar Origem** permite escolher outro ficheiro para a ligação escolhida, por exemplo quando o livro foi movido. As fórmulas mantêm-se como estão. O novo local é escrito no ficheiro quando guardar, e **Anular** desfaz a alteração.
- **Abrir Origem** abre a origem escolhida numa nova janela, quando pode ser encontrada.
- **Quebrar Ligação** transforma cada célula que lê a origem escolhida no valor que mostra agora, e um nome que representava um local no seu interior em **#REF!**. O JeffCrunch pergunta primeiro. **Quebrar Ligação** é um passo, pelo que **Anular** repõe as fórmulas, os nomes e as ligações, até guardar o livro. Quando não resta nenhuma ligação, o ficheiro é guardado sem as partes que as continham.

## Formulário

**Formulário** (no grupo **Ferramentas de Dados** e na caixa de pesquisa) mostra uma linha de uma lista de cada vez, como um pequeno formulário. Uma lista é uma tabela, ou um bloco de células com uma linha de nomes no topo e uma linha de dados por baixo de cada nome. Clique primeiro em qualquer célula da lista.

- Cada caixa corresponde a uma coluna, com o nome da linha superior. Escreva numa caixa para alterar o valor. Uma caixa a cinzento contém uma fórmula: o formulário mostra o resultado, e a fórmula é alterada na folha.
- **Localizar Seguinte** e **Localizar Anterior** passam para a linha seguinte e para a anterior. Mudar para outra linha, ou premir **Enter**, escreve o que escreveu nesta linha.
- **Novo** mostra um formulário vazio. Preencha-o e prima **Novo** novamente, **Localizar Seguinte** ou **Fechar**. A linha é acrescentada abaixo da última linha da lista. Uma tabela cresce uma linha. Se uma célula por baixo da lista não estiver vazia, o formulário indica-o e não acrescenta nada.
- **Eliminar** retira a linha que está a ver, depois de pedir confirmação. As linhas abaixo sobem. Uma tabela mantém sempre pelo menos uma linha de dados. Uma linha oculta, ou que um filtro oculta, não é mostrada no formulário, não é contada e nunca é eliminada por ele. Se a lista for demasiado longa para ser percorrida inteira, o formulário indica que mostra apenas as primeiras linhas.
- **Restaurar** desfaz o que escreveu nesta linha, antes de a deixar.
- **Critérios** permite procurar linhas. Escreva uma condição numa ou mais caixas e prima **Localizar Seguinte** ou **Localizar Anterior**. Uma condição é um valor, como **Ana** (o texto corresponde ao início da palavra, e **?** e ***** representam carateres), ou um valor depois de um sinal: **>100**, **<=5**, **<>0**, **=Oslo**. Uma linha tem de cumprir todas as condições que escreveu. Prima **Formulário** para voltar, ou **Limpar** para recomeçar. Enquanto houver critérios definidos, **Localizar Seguinte** e **Localizar Anterior** saltam as linhas que não os cumprem.
- **Fechar** escreve a linha e fecha o formulário.

Cada linha que altera, acrescenta ou elimina é um passo, pelo que **Anular** na folha a desfaz de uma só vez.

## Ordenar e filtrar

O grupo **Ordenar e Filtrar** tem os mesmos comandos do menu **Ordenar e Filtrar Linhas** do separador Base. Consulte [Ordenar e filtrar](sort-filter).

## Texto para Colunas

Use esta opção quando uma coluna contém várias partes de texto que pretende ter em colunas separadas, como "Silva, Ana, 31".

1. Selecione as células de **uma** coluna.
2. Escolha **Texto para Colunas** no separador **Dados**.
3. Escolha como dividir o texto:
   - **Dividir num separador**: marque os sinais que ficam entre as partes: **Tabulação**, **Ponto e vírgula**, **Vírgula**, **Espaço** ou **Outro**, onde escreve o seu próprio sinal. **Tratar separadores consecutivos como um só** impede que duas vírgulas seguidas criem uma parte vazia. **Texto entre aspas** indica o sinal que envolve uma parte que não deve ser dividida, como "Lee, Bo". Dentro das aspas, dois sinais seguidos representam um só.
   - **Dividir em posições fixas**: escreva as posições onde começa uma nova coluna, contando os carateres a partir de 1. Por exemplo, 5, 12 começa uma nova coluna no quinto e no décimo segundo carácter.
4. Escolha o que é cada parte: **Geral** lê números e datas como se os tivesse escrito, e **Texto** mantém cada parte tal como está, pelo que "007" continua "007".
   Para tratar uma parte de forma diferente, escreva o seu número em **Parte** e escolha como é lida: **Igual ao anterior**, **Como escrito**, **Texto**, uma data escrita como **mês, dia, ano**, **dia, mês, ano**, ou **ano, mês, dia**, ou **Omitir**, que não coloca essa parte em lado nenhum. A primeira parte é a 1. Uma parte de data pode ter o mês por extenso (**5 mar 2024**, **5 de março de 2024**) em inglês ou no seu idioma, pode estar separada por /, -, um ponto, uma vírgula ou espaços, e pode ser seguida de uma hora (**10:30**, **10:30:45** ou **10:30 PM**). Uma parte que não seja uma data na ordem escolhida (um mês 13, um dia que não existe, um ano que não tem dois nem quatro algarismos, ou uma hora que não faz sentido) é mantida como texto, tal como foi escrita; nunca é lida noutra ordem.
5. Em **Colocar as partes a partir de**, escreva a primeira célula para as partes. Começa como a primeira célula que selecionou.
6. A caixa por baixo das opções mostra as primeiras linhas divididas da forma que escolheu. Prima **OK**.

As partes são colocadas nas células à direita da primeira célula, e substituem o que lá estava. **Texto para Colunas** é um passo que pode anular.

## Remover Duplicados

Use esta opção para manter apenas a primeira de cada grupo de linhas repetidas.

1. Selecione as células da lista, ou uma célula dentro dela. O JeffCrunch usa todo o bloco de dados à volta de uma única célula.
2. Escolha **Remover Duplicados**.
3. Marque **Os meus dados têm uma linha de cabeçalho** se a primeira linha tiver nomes. Essa linha fica e não é comparada.
4. Marque as colunas que têm de coincidir para que duas linhas contem como repetidas. Duas linhas só se repetem quando todas as colunas marcadas são iguais. As letras maiúsculas e minúsculas contam como iguais, e uma fórmula conta pelo valor que dá.
5. Prima **OK**. A caixa indica quantas linhas foram removidas e quantas restam.

As linhas abaixo de uma linha removida sobem dentro das células selecionadas, com as suas formatações e fórmulas. As células abaixo da seleção não se movem. As linhas vazias que ficam no fim da seleção são limpas. **Remover Duplicados** é um passo que pode anular.

## Atingir Objetivo

Use esta opção quando sabe a resposta que pretende, mas não o número que a dá. Por exemplo, uma célula de pagamento tem uma fórmula que usa uma taxa de juro, e pretende que o pagamento seja 500. O Atingir Objetivo experimenta números na célula da taxa até o pagamento ser 500.

1. Escolha **Atingir Objetivo** no separador **Dados**.
2. Em **Célula com a fórmula**, escreva a célula que contém a fórmula, como B3. Começa como a célula que selecionou.
3. Em **Valor pretendido**, escreva a resposta que pretende que a fórmula dê.
4. Em **Célula a alterar**, escreva a célula que o Atingir Objetivo pode alterar, como B1. Tem de conter um número, não uma fórmula.
5. Prima **OK**.

Se o Atingir Objetivo encontrar um número, este fica na célula a alterar, e a barra de estado indica qual é. Se não o conseguir, nada muda e é avisado. Experimente outro número inicial na célula a alterar, e confirme que a fórmula usa realmente essa célula. **Atingir Objetivo** é um passo que pode anular.

## Gestor de Cenários

Um cenário é um conjunto com nome de valores para algumas células. Use cenários para experimentar "e se" numa só folha. Por exemplo, mantenha um cenário "Vendas baixas" e um cenário "Vendas altas" para as mesmas células de entrada, e mostre aquele que pretende ver.

1. Escolha **Gestor de Cenários** no separador **Dados**.
2. Prima **Adicionar**. Escreva um nome, como Vendas altas. Em **Células a alterar**, escreva as células que o cenário define, como B1:B3 ou D5. Um cenário pode alterar até 32 células. As células que selecionou são preenchidas por si. Cada célula começa com o valor que tem agora. Escreva na tabela o valor que este cenário deve usar, e prima **OK**.
3. Escolha um cenário na lista e prima **Mostrar** para colocar os seus valores nas células. Clicar duas vezes num nome faz o mesmo. Prima **Editar** para alterar o seu nome, o seu comentário ou os seus valores. Prima **Eliminar** para o retirar.

**Mostrar**, **Adicionar**, **Editar** e **Eliminar** são cada um um passo que pode anular.

**Resumo** coloca todos os cenários lado a lado numa nova folha chamada Resumo de Cenário. Pede as células de resultado, que são as células que pretende comparar, como o total em B10. Para cada cenário, o resumo mostra os valores das células a alterar e o que as células de resultado dão, calculado com os valores desse cenário. Uma coluna chamada Valores Atuais mostra como estavam as células quando fez o resumo. As suas próprias células são deixadas como estavam.

Os cenários são guardados no livro. Um livro feito por outro programa mantém os seus cenários, e aqui pode vê-los. Uma cópia guardada como ficheiro .xls mais antigo ou como .ods não os contém, e a gravação avisa disso.

## Subtotal

Use esta opção para colocar uma linha de totais abaixo de cada grupo de linhas que têm a mesma entrada numa coluna, e um total geral no fim. A lista precisa de uma linha de cabeçalho e deve estar ordenada pela coluna pela qual agrupa, para que as entradas iguais fiquem umas junto das outras.

1. Selecione uma célula da lista.
2. Escolha **Subtotal** no grupo **Estrutura de Tópicos** do separador **Dados**.
3. Em **A cada alteração em**, escolha a coluna que agrupa as linhas, como Região.
4. Em **Utilizar função**, escolha o que calcular: **Soma**, **Contagem**, **Média**, **Máx** e **Mín**, entre outras.
5. Em **Adicionar subtotal a**, marque cada coluna a totalizar.
6. Deixe **Substituir subtotais atuais** marcado para substituir os subtotais que já existem.
7. **Resumo abaixo dos dados** começa como a estrutura de tópicos da folha o tem. Deixe-o marcado para colocar cada linha de total abaixo do seu grupo, ou desmarque-o para colocar cada linha de total acima do seu grupo, com o total geral no topo. Prima **OK**.

Cada linha de total diz "Total Leste" (ou a entrada a que se refere) e tem uma fórmula SUBTOTAL, pelo que deixa de fora as outras linhas de total. As linhas de cada grupo ficam agrupadas, pelo que pode usar os botões 1, 2 e 3 ao lado das linhas para mostrar apenas os totais. Para retirar os totais, abra **Subtotal** novamente e prima **Remover Tudo**. Se os totais passaram para o outro lado de uma folha que já tinha os seus próprios grupos, a barra de estado indica que esses grupos têm agora as suas linhas de resumo nesse lado, e Remover Tudo repõe o lado como estava antes do primeiro subtotal, a menos que o tenha alterado manualmente desde então. Quando os totais estão acima, os botões da estrutura ficam junto às linhas de total, pelo que um botão de mais ou de menos ao lado de "Total Leste" mostra ou oculta as linhas de Leste abaixo dele. Se desmarcar **Substituir subtotais atuais**, os novos totais têm de ficar do mesmo lado dos que já existem. Caso contrário, nada é alterado e uma mensagem indica-o. Acrescentar ou remover subtotais é um passo que pode anular.

## Filtro Avançado

Os botões de Filtro escolhem linhas uma coluna de cada vez. **Avançado** (no grupo **Ordenar e Filtrar**) permite escrever as condições na folha, pelo que podem ser tão longas quanto precisar.

1. Junto à lista, copie os cabeçalhos das colunas que pretende testar para uma linha de células vazias.
2. Por baixo de cada cabeçalho, escreva uma condição. As condições na **mesma linha** têm todas de ser verdadeiras (é o "e"). As condições em **linhas diferentes** são alternativas: uma linha da lista tem de cumprir apenas uma dessas linhas (é o "ou").
3. Escolha **Avançado**. Confirme que **Intervalo da lista** é a lista com a sua linha de cabeçalho, e escreva as células dos seus cabeçalhos e condições em **Intervalo das condições**.
4. Escolha **Ocultar as outras linhas, na lista**, ou **Copiar as linhas para outro local** e escreva a primeira célula do local, como H1. Marque **Omitir linhas que repetem uma anterior** para copiar apenas uma de cada.
5. Prima **OK**.

Como escrever uma condição:

- Uma palavra, como **An**, escolhe texto que começa por ela. As letras maiúsculas e minúsculas contam como iguais. **?** representa qualquer carácter e ***** qualquer sequência de carateres.
- Um número escolhe células que lhe são iguais.
- **=Ana** escolhe texto que é exatamente Ana. **=** sozinho escolhe células vazias.
- **>**, **>=**, **<**, **<=** e **<>** (diferente) antes de um número ou de uma palavra comparam com ele, como **>25**. **<>** sozinho escolhe células que não estão vazias.

Uma linha de condições sem nada escrito escolhe todas as linhas. Para voltar a mostrar as linhas ocultas, escolha **Limpar**. Uma cópia é feita de imediato e não muda quando a lista muda. **Filtro Avançado** é um passo que pode anular.

## Validação de Dados

Use esta opção para indicar o que pode ser escrito em algumas células, para que uma entrada errada seja impedida ou questionada.

1. Selecione as células.
2. Escolha **Validação de Dados** no grupo **Ferramentas de Dados** do separador **Dados**.
3. No separador **Definições**, escolha o que **Permitir**: **Qualquer valor** (sem regra), **Número inteiro**, **Decimal**, **Lista**, **Data**, **Hora**, **Comprimento do texto** ou **Personalizado**.
4. Para um número, uma data, uma hora ou um comprimento de texto, escolha **Dados** (**entre**, **não está entre**, **igual a**, e assim por diante) e escreva os limites. Um limite pode ser um valor ou uma fórmula que começa por =.
5. Para uma **Lista**, escreva as opções separadas por vírgulas, como Vermelho,Verde,Azul, ou escreva um intervalo de células depois de =, como =$A$1:$A$5. A célula passa a mostrar uma seta com as opções. Desative **Lista pendente na célula** para ocultar a seta.
6. Para **Personalizado**, escreva uma fórmula que começa por =. A entrada é permitida quando a fórmula for verdadeira.
7. **Ignorar em branco** deixa passar uma célula vazia.

No separador **Mensagem de Entrada**, marque a caixa e escreva um título e uma mensagem. Aparecem junto à célula sempre que ela estiver selecionada.

No separador **Alerta de Erro**, escolha o que acontece a uma entrada não permitida. **Parar** recusa-a. **Aviso** pergunta se a deve manter. **Informação** avisa a pessoa e mantém-na. Escreva um título e uma mensagem para o alerta.

**Assinalar Dados Inválidos** (junto a **Validação de Dados**) coloca um círculo vermelho em cada célula da folha cujo valor a sua regra não permite, como um número que lá estava antes da regra, ou o resultado de uma fórmula. Uma mensagem indica se todas as células estão corretas. Uma célula que corrija perde o seu círculo. **Limpar Assinalações de Validação** retira todos os círculos.

**Limpar Tudo** retira as regras das células selecionadas. A nova regra substitui qualquer regra que as células selecionadas tivessem; as outras células mantêm as suas. Alterar regras é um passo que pode anular, e as regras são guardadas no ficheiro.

## Consolidar

Use esta opção para combinar intervalos semelhantes, como a mesma tabela de várias folhas, numa só tabela.

1. Escolha **Consolidar** no grupo **Ferramentas de Dados** do separador **Dados**.
2. Escolha a **Função** a usar na combinação, como **Soma** ou **Média**.
3. Escreva um intervalo na caixa e prima **Adicionar**. Use **Folha2!A1:C10** para um intervalo noutra folha. Adicione cada intervalo que pretende combinar. Selecione um intervalo na lista e prima **Eliminar** para o retirar.
4. Se os intervalos tiverem nomes na primeira linha, marque **A linha superior contém os nomes das colunas**. Se tiverem nomes na primeira coluna, marque **A coluna da esquerda contém os nomes das linhas**.
5. Em **Colocar o resultado em**, escreva a primeira célula do resultado.
6. Marque **Criar ligações aos dados de origem** se cada resultado tiver de ser uma fórmula que lê as suas células de origem, para que mude quando elas mudarem. Sem esta opção, os resultados são números simples.
7. Prima **OK**.

Sem nomes marcados, combina-se a célula que está no mesmo lugar em cada intervalo (a primeira célula de cada intervalo, depois a segunda, e assim por diante). Com os nomes marcados, combinam-se as células com os mesmos nomes, onde quer que estejam em cada intervalo, e um nome que só alguns intervalos têm continua a ganhar uma linha ou coluna. **Consolidar** é um passo que pode anular.

## Folha de Previsão

A Folha de Previsão cria uma nova folha com uma tabela e um gráfico de valores futuros, calculados a partir de uma linha temporal e de uma série de valores. Encontra a tendência e a época que se repete nos valores, e acrescenta uma faixa dentro da qual os valores futuros provavelmente ficarão.

1. Selecione a linha temporal e os valores, duas colunas lado a lado (a linha temporal primeiro). A linha temporal são datas ou números com a mesma distância entre si, como todos os meses ou de 5 em 5 unidades. Uma primeira linha de nomes fica de fora.
2. Escolha **Folha de Previsão** no grupo **Previsão** do separador **Dados**.
3. Uma imagem mostra a previsão e altera-se à medida que altera as opções. Escolha **Gráfico de linhas** ou **Gráfico de colunas**.
4. Defina as opções:
   - **Intervalo da linha temporal** e **Intervalo de valores** são as células a ler. Escreva aqui outras células se a seleção estiver errada.
   - **Início da previsão** fica vazio para começar depois do último ponto. Uma data ou número anterior ao último ponto começa aí e usa apenas os pontos anteriores, para que possa comparar a previsão com o que aconteceu.
   - **Fim da previsão** é a última data a prever. São previstos no máximo 10 000 pontos.
   - **Intervalo de confiança** indica quão certa é a faixa, começando em 95%.
   - **Sazonalidade** é encontrada ao analisar os valores, ou pode definir o comprimento da época em pontos, ou escolher nenhuma. Os valores têm de conter duas épocas completas.
   - **Preencher pontos em falta com** indica o que recebe um momento sem valor: uma linha reta entre os vizinhos, ou zero.
   - **Agregar duplicados utilizando** indica como se combinam os valores no mesmo momento: Média, Contagem, CountA, Máx, Mediana, Mín ou Soma.
   - **Incluir estatísticas de previsão** acrescenta uma tabela com Alfa, Beta, Gama, MASE, SMAPE, MAE e RMSE, que indicam o ajuste da previsão aos valores.
5. Se algo estiver errado, a caixa diz o quê em linguagem simples (passos que não são iguais, pontos insuficientes, texto nos valores), e **Criar** fica à espera.
6. Prima **Criar**.

A nova folha é colocada antes da que estava a usar. A sua tabela tem as colunas Linha temporal, Valores, Previsão, Limite inferior de confiança e Limite superior de confiança. A Previsão e os limites são fórmulas que usam `PREVISÃO.ETS` e `PREVISÃO.ETS.CONFINT` sobre as próprias colunas de Linha temporal e Valores da tabela, pelo que são recalculadas quando altera um valor. O gráfico mostra os valores, a previsão e a faixa. A Folha de Previsão é um passo que pode anular.

Os números podem diferir em alguns por cento dos de outros programas de folhas de cálculo, que usam a sua própria forma de ajustar a suavização, e uma previsão que um ficheiro já contenha mantém os valores que o ficheiro guardou até os dados que lê serem alterados.

Um ficheiro com mais de 1 000 000 de pontos não é usado.
