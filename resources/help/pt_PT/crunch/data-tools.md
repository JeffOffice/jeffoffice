# Ferramentas de dados
<!-- keywords: previsão, folha de previsão, valores futuros, intervalo de confiança, sazonalidade, tendência, suavização, formulário, formulário de dados, introdução de dados, registo, registos, novo registo, localizar seguinte, localizar anterior, critérios, gestor de cenários, cenários, e se, e se, resumo de cenário, dados, separador dados, texto para colunas, dividir, separador, vírgula, tabulação, largura fixa, remover duplicados, atingir objetivo, subtotal, totais, filtro avançado, condições, validação de dados, assinalar dados inválidos, círculo vermelho, limpar assinalações de validação, consolidar, combinar intervalos, permitir, lista, lista pendente, mensagem de entrada, alerta de erro, linhas repetidas, único, ficheiro de texto, csv, de texto, colunas, delimitador, aspas, editar ligações, atualizar valores, alterar origem, ligações, externo, origem, quebrar ligação, verificar estado, abrir origem -->

O separador **Dados** tem as ferramentas que trabalham sobre uma lista de valores: obter texto de um ficheiro, ordenar uma lista, dividir texto em colunas e eliminar linhas repetidas.

## De Texto/CSV

**De Texto/CSV** abre um ficheiro de texto (.csv, .tsv ou .txt). É o mesmo que **Abrir Ficheiro de Texto** no separador Ficheiro. O JeffCrunch pergunta como o ficheiro está dividido em colunas e que tipo de texto contém, e depois mostra-o como um novo livro.

## De Ficheiro

**De Ficheiro**, no grupo **Obter Dados**, carrega as linhas de um ficheiro JSON, de um ficheiro XML ou de outro livro como tabela. Só é possível carregar um ficheiro deste computador. Um endereço Web ou um ficheiro de outro computador é recusado.

1. Clique em **De Ficheiro**, depois em **Procurar** e escolha o ficheiro. As primeiras linhas aparecem de imediato, com o tipo que o JeffCrunch encontrou para cada coluna, com base nessas primeiras linhas: número, data, verdadeiro ou falso, ou texto. O ficheiro inteiro é verificado ao ser carregado, por isso uma coluna pode acabar como texto. Nada na folha muda até clicar em **Carregar**.
2. Quando o ficheiro tem mais do que uma lista, escolha a que deseja carregar. Num ficheiro JSON é uma lista de itens com valores com nome, como `$.items`. Num ficheiro XML é um item que se repete, como `/feed/entry`. Num livro é uma folha, uma tabela ou um intervalo com nome.
3. Um item JSON dentro de outro item torna-se uma coluna chamada `externo.interno`, até quatro níveis. Uma lista de valores simples fica juntada numa única célula (os primeiros 10 000 valores), ou distribuída por colunas (as primeiras 16), conforme escolher. Em XML, os atributos e os itens filhos tornam-se colunas. Dois valores diferentes nunca partilham uma coluna: quando dois ficariam com o mesmo nome, o segundo recebe um número acrescentado ou, em XML, o seu prefixo, e uma nota indica-o. Um nome repetido dentro de um item JSON mantém o último valor, e uma nota indica-o. Um número demasiado grande ou demasiado pequeno para uma célula é mantido como texto, tal como está escrito, e um carácter que não pode ser lido é mostrado com uma marca de substituição; ambos são assinalados em nota.
4. Marque **Carregar todas as colunas como texto** para manter cada valor tal como está escrito. Uma coluna de números com zero à esquerda, como 007, é sempre mantida como texto.
5. Escreva a célula onde a tabela fica e clique em **Carregar**. Se a tabela cobrir células que contêm algo, o JeffCrunch pergunta antes de as substituir. Se não houver espaço, ou se a folha estiver protegida, é avisado e nada muda.

A tabela é um passo de **Anular**. Guarda o ficheiro e as suas escolhas. Para ler o ficheiro novamente, clique numa célula da tabela e clique em **Atualizar** no separador Dados. **Atualizar Tudo** é o botão grande ao lado: lê todas as tabelas criadas por Obter Dados e, depois, atualiza todas as tabelas dinâmicas, tudo como um único passo de Anular. A tabela cresce ou diminui conforme as linhas do ficheiro. As colunas que acrescentou ao lado da tabela mantêm-se: uma fórmula preenchida para baixo é preenchida nas novas linhas, e as linhas que deixaram de existir no ficheiro também saem. Se o ficheiro já não existir ou não puder ser lido, é avisado e nada muda. Se as colunas do próprio ficheiro mudaram, é informado de quais estão em falta ou são novas, e nada muda. Crie a tabela de novo com Obter Dados, ou repone as colunas no ficheiro.

A ligação ao ficheiro é guardada na tabela, num livro `.xlsx` gravado, numa parte que outros programas ignoram. A ligação contém o caminho completo do ficheiro neste computador, por isso quem receber o livro pode ler esse caminho. Para o retirar antes de partilhar o livro, use **Inspecionar**, depois **Ligações a ficheiros de dados** e depois **Remover**. Uma cópia guardada noutro tipo de ficheiro não a mantém, e **Verificar Compatibilidade** indica-o. **Inspecionar** lista as ligações em **Ligações a ficheiros de dados** e pode removê-las. As tabelas e as suas células mantêm-se.

Os ficheiros são lidos com limites: um ficheiro até 64 MB, 200 000 linhas, 256 colunas e 32 767 carateres por célula. XML com declaração de tipo de documento ou entidades é recusado. Tudo o que for cortado por um limite é indicado.

## Editar Ligações

Se um livro tiver fórmulas que leem outro livro (como ='[Preços.xlsx]Folha1'!A1), **Editar Ligações**, no grupo **Obter Dados**, lista esses outros livros, que se chamam origens. Cada linha mostra onde está a origem e o respetivo estado. Até o livro ser aberto novamente com as suas origens, mostra os números que o seu ficheiro guardou a partir delas.

- **Verificar Estado** procura cada origem neste computador e indica **Encontrada**, ou indica que não foi encontrada. Uma origem na Web não é obtida, por isso o estado indica que não foi verificada.
- **Atualizar Valores** lê a origem escolhida deste computador e dá a cada célula que é apenas uma referência a uma das suas células o valor que essa célula tem agora. Uma célula que faz mais com o valor, como `=[1]Preços!A1*2`, mantém o número que o ficheiro guardou, e a mensagem por baixo da lista indica quantas células foram atualizadas e quantas não foram. **Atualizar Valores** é um único passo, por isso **Anular** repõe os valores antigos. Os novos valores são também escritos na parte da ligação dentro do ficheiro ao guardar, para que o livro os mostre da próxima vez que for aberto, mesmo quando a origem não estiver presente.
- **Alterar Origem** permite escolher outro ficheiro para a ligação escolhida, por exemplo quando o livro foi movido. As fórmulas mantêm-se como estão. O novo local é escrito no ficheiro ao guardar, e **Anular** desfaz a alteração.
- **Abrir Origem** abre a origem escolhida numa nova janela, quando for possível encontrá-la.
- **Quebrar Ligação** transforma cada célula que lê a origem escolhida no valor que mostra agora, e um nome que representava um local nela passa a **#REF!**. O JeffCrunch pede confirmação antes. **Quebrar Ligação** é um único passo, por isso **Anular** repõe as fórmulas, os nomes e as ligações, até guardar o livro. Quando não resta nenhuma ligação, o ficheiro é guardado sem as partes que as continham.

## Formulário

**Formulário** (no grupo **Ferramentas de Dados** e na caixa de pesquisa) mostra uma linha de cada vez de uma lista, como um pequeno formulário. Uma lista é uma tabela, ou um bloco de células com uma linha de nomes no topo e uma linha de dados por baixo de cada nome. Clique primeiro em qualquer célula da lista.

- Cada caixa corresponde a uma coluna, identificada com o nome da linha superior. Escreva numa caixa para alterar o valor. Uma caixa esbatida contém uma fórmula: o formulário mostra o resultado, e a fórmula é alterada na folha.
- **Localizar Seguinte** e **Localizar Anterior** passam para a linha seguinte e para a anterior. Ao mudar para outra linha, ou ao premir **Enter**, o que escreveu nesta linha é gravado.
- **Novo** mostra um formulário vazio. Preencha-o e depois prima **Novo** outra vez, **Localizar Seguinte** ou **Fechar**. A linha é acrescentada abaixo da última linha da lista. Uma tabela cresce uma linha. Se uma célula abaixo da lista não estiver vazia, o formulário avisa e não acrescenta nada.
- **Eliminar** retira a linha que está a ver, depois de pedir confirmação. As linhas abaixo dela sobem. Uma tabela mantém sempre pelo menos uma linha de dados. Uma linha oculta, ou que um filtro oculta, não é mostrada no formulário, não é contada e nunca é eliminada por ele. Se a lista for demasiado longa para ser percorrida inteira, o formulário indica que mostra apenas as primeiras linhas.
- **Restaurar** repõe o que escreveu nesta linha, antes de sair dela.
- **Critérios** permite procurar linhas. Escreva uma condição numa ou em várias caixas e depois prima **Localizar Seguinte** ou **Localizar Anterior**. Uma condição é um valor, como **Ana** (o texto corresponde ao início da palavra, e **?** e ***** representam letras), ou um valor precedido de um sinal: **>100**, **<=5**, **<>0**, **=Oslo**. Uma linha tem de cumprir todas as condições que escrever. Prima **Formulário** para voltar, ou **Limpar** para recomeçar. Enquanto houver critérios definidos, **Localizar Seguinte** e **Localizar Anterior** saltam as linhas que não os cumprem.
- **Fechar** grava a linha e fecha o formulário.

Cada linha que altera, acrescenta ou elimina é um passo, por isso **Anular** na folha a desfaz de uma só vez.

## Ordenar e filtrar

O grupo **Ordenar e Filtrar** tem os mesmos comandos do menu **Ordenar e Filtrar Linhas** no separador Base. Veja [Ordenar e filtrar](sort-filter).

## Texto para Colunas

Use isto quando uma coluna contém vários pedaços de texto que deseja ter em colunas separadas, como "Silva, Ana, 31".

1. Selecione as células de **uma** coluna.
2. Escolha **Texto para Colunas** no separador **Dados**.
3. Escolha como dividir o texto:
   - **Dividir num separador**: marque os sinais que existem entre os pedaços: **Tabulação**, **Ponto e vírgula**, **Vírgula**, **Espaço** ou **Outro**, onde escreve o seu próprio sinal. **Tratar separadores consecutivos como um só** evita que duas vírgulas seguidas criem um pedaço vazio. **Texto entre aspas** indica o sinal que envolve um pedaço que não deve ser dividido, como "Lee, Bo". Dentro das aspas, dois sinais seguidos valem como um.
   - **Dividir em posições fixas**: escreva as posições onde começa uma nova coluna, contando os carateres a partir de 1. Por exemplo, 5, 12 começa uma nova coluna no quinto e no décimo segundo carácter.
4. Escolha o que é cada pedaço: **Geral** lê números e datas como se os tivesse escrito, e **Texto** mantém cada pedaço tal como está, por isso "007" continua a ser "007".
   Para tratar um pedaço de forma diferente, escreva o seu número em **Pedaço** e escolha como é lido: **Igual ao anterior**, **Tal como escrito**, **Texto**, uma data escrita como **mês, dia, ano**, **dia, mês, ano** ou **ano, mês, dia**, ou **Omitir**, que não coloca esse pedaço em lado nenhum. O primeiro pedaço é o 1. Um pedaço de data pode ter o mês por extenso (**5 mar 2024**, **5 de março de 2024**) em português ou noutro idioma, pode ser separado por /, -, ponto, vírgula ou espaços, e pode ser seguido de uma hora (**10:30**, **10:30:45** ou **10:30 PM**). Um pedaço que não seja uma data na ordem escolhida (um mês 13, um dia que não existe, um ano que não tem dois ou quatro algarismos, ou uma hora sem sentido) é mantido como texto, tal como foi escrito; nunca é lido noutra ordem.
5. Em **Colocar os pedaços a partir de**, escreva a primeira célula para os pedaços. Começa como a primeira célula que selecionou.
6. A caixa por baixo das opções mostra as primeiras linhas divididas como escolheu. Prima **OK**.

Os pedaços vão para as células à direita da primeira célula, e substituem o que lá estava. **Texto para Colunas** é um passo que pode anular.

## Remover Duplicados

Use isto para manter apenas a primeira de cada grupo de linhas repetidas.

1. Selecione as células da lista, ou uma célula dentro dela. O JeffCrunch toma o bloco inteiro de dados à volta de uma única célula.
2. Escolha **Remover Duplicados**.
3. Marque **Os meus dados têm linha de cabeçalho** se a primeira linha contiver nomes. Essa linha mantém-se e não é comparada.
4. Marque as colunas que têm de coincidir para que duas linhas contem como repetidas. Duas linhas só se repetem quando todas as colunas marcadas são iguais. Maiúsculas e minúsculas contam como o mesmo, e uma fórmula conta como o valor que devolve.
5. Prima **OK**. A caixa indica quantas linhas foram removidas e quantas restam.

As linhas abaixo de uma linha removida sobem dentro das células selecionadas, com os respetivos formatos e fórmulas. As células abaixo da seleção não se movem. As linhas vazias que ficam no fim da seleção são limpas. **Remover Duplicados** é um passo que pode anular.

## Atingir Objetivo

Use isto quando conhece a resposta que quer, mas não o número que a dá. Por exemplo, uma célula de pagamento tem uma fórmula que usa uma taxa de juro, e quer que o pagamento seja 500. O Atingir Objetivo experimenta números na célula da taxa até o pagamento ser 500.

1. Escolha **Atingir Objetivo** no separador **Dados**.
2. Em **Célula com a fórmula**, escreva a célula que contém a fórmula, como B3. Começa como a célula que selecionou.
3. Em **Valor pretendido**, escreva a resposta que quer que a fórmula dê.
4. Em **Célula a alterar**, escreva a célula que o Atingir Objetivo pode alterar, como B1. Tem de conter um número, não uma fórmula.
5. Prima **OK**.

Se o Atingir Objetivo encontrar um número, este fica na célula a alterar, e a barra de estado indica qual é. Se não conseguir, nada muda e é avisado. Experimente outro número inicial na célula a alterar, e verifique se a fórmula usa mesmo essa célula. **Atingir Objetivo** é um passo que pode anular.

## Gestor de Cenários

Um cenário é um conjunto com nome de valores para algumas células. Use cenários para experimentar "e se" numa só folha. Por exemplo, mantenha um cenário "Vendas baixas" e um cenário "Vendas altas" para as mesmas células de entrada, e mostre aquele que quer analisar.

1. Escolha **Gestor de Cenários** no separador **Dados**.
2. Prima **Adicionar**. Escreva um nome, como Vendas altas. Em **Células a alterar**, escreva as células que o cenário define, como B1:B3 ou D5. Um cenário pode alterar até 32 células. As células que selecionou são preenchidas por si. Cada célula começa com o valor que tem agora. Escreva na tabela o valor que este cenário deve usar, e prima **OK**.
3. Escolha um cenário na lista e prima **Mostrar** para colocar os seus valores nas células. Clicar duas vezes num nome faz o mesmo. Prima **Editar** para alterar o nome, o comentário ou os valores. Prima **Eliminar** para o retirar.

**Mostrar**, **Adicionar**, **Editar** e **Eliminar** são, cada um, um passo que pode anular.

**Resumo** coloca todos os cenários lado a lado numa nova folha chamada Resumo de Cenário. Pede as células de resultado, que são as células que quer comparar, como o total em B10. Para cada cenário, o resumo mostra os valores das células alteradas e o resultado das células de resultado, calculado com os valores desse cenário. Uma coluna chamada Valores Atuais mostra como estavam as células quando fez o resumo. As suas próprias células ficam como estavam.

Os cenários são guardados no livro. Um livro feito noutro programa mantém os seus cenários, e aqui pode vê-los. Uma cópia guardada como ficheiro .xls mais antigo ou como .ods não os contém, e o guardar indica-o.

## Subtotal

Use isto para colocar uma linha de totais abaixo de cada grupo de linhas que têm a mesma entrada numa coluna, e um total geral no fim. A lista precisa de uma linha de cabeçalho e deve estar ordenada pela coluna pela qual agrupa, para que entradas iguais fiquem umas ao lado das outras.

1. Selecione uma célula da lista.
2. Escolha **Subtotal** no grupo **Estrutura de Tópicos** do separador **Dados**.
3. Em **Em cada alteração em**, escolha a coluna que agrupa as linhas, como Região.
4. Em **Usar função**, escolha o que calcular: **Soma**, **Contar**, **Média**, **Máx.**, **Mín.** e outras.
5. Em **Adicionar subtotal a**, marque cada coluna a totalizar.
6. Deixe marcado **Substituir subtotais atuais** para substituir subtotais que já existam.
7. **Resumo abaixo dos dados** começa como a estrutura de tópicos da folha o tem. Deixe-o marcado para colocar cada linha de total abaixo do seu grupo, ou desmarque-o para colocar cada linha de total acima do seu grupo, com o total geral no topo. Prima **OK**.

Cada linha de total diz "Total Leste" (ou a entrada a que diz respeito) e tem uma fórmula SUBTOTAL, por isso deixa de fora as outras linhas de total. As linhas de cada grupo ficam agrupadas, para que possa usar os botões 1, 2 e 3 ao lado das linhas para mostrar apenas os totais. Para retirar os totais, abra **Subtotal** novamente e prima **Remover Tudo**. Se os totais passaram para o outro lado de uma folha que já tinha os seus próprios grupos, a barra de estado indica que esses grupos têm agora as linhas de resumo desse lado, e Remover Tudo repõe o lado como estava antes do primeiro subtotal, a menos que o tenha alterado entretanto. Quando os totais estão acima, os botões da estrutura ficam nas linhas de total, por isso um botão de mais ou de menos ao lado de "Total Leste" mostra ou oculta as linhas Leste abaixo dele. Se desativar **Substituir subtotais atuais**, os novos totais têm de ficar do mesmo lado dos que já existem. Caso contrário, nada é alterado, e uma mensagem indica-o. Acrescentar ou remover subtotais é um passo que pode anular.

## Filtro Avançado

Os botões de filtro escolhem linhas uma coluna de cada vez. **Avançado** (no grupo **Ordenar e Filtrar**) permite escrever as condições na folha, para que sejam tão longas quanto precisar.

1. Ao lado da lista, copie os cabeçalhos das colunas que quer testar para uma linha de células vazias.
2. Por baixo de cada cabeçalho, escreva uma condição. As condições na **mesma linha** têm de ser todas verdadeiras (é o "e"). As condições em **linhas diferentes** são alternativas: uma linha da lista precisa de cumprir apenas uma dessas linhas (é o "ou").
3. Escolha **Avançado**. Confirme que o **Intervalo da lista** é a lista com a sua linha de cabeçalho, e escreva as células dos seus cabeçalhos e condições em **Intervalo das condições**.
4. Escolha **Ocultar as outras linhas, na lista**, ou **Copiar as linhas para outro local** e escreva a primeira célula do local, como H1. Marque **Omitir linhas que repetem uma anterior** para copiar apenas uma de cada.
5. Prima **OK**.

Como escrever uma condição:

- Uma palavra, como **An**, escolhe texto que começa por ela. Maiúsculas e minúsculas contam como o mesmo. **?** representa qualquer carácter e ***** qualquer sequência de carateres.
- Um número escolhe células que lhe são iguais.
- **=Ana** escolhe texto que é exatamente Ana. **=** sozinho escolhe células vazias.
- **>**, **>=**, **<**, **<=** e **<>** (diferente de) antes de um número ou de uma palavra comparam com ele, como **>25**. **<>** sozinho escolhe células que não estão vazias.

Uma linha de condições sem nada escrito escolhe todas as linhas. Para voltar a mostrar as linhas ocultas, escolha **Limpar**. Uma cópia é feita de imediato e não muda quando a lista muda. **Filtro Avançado** é um passo que pode anular.

## Validação de Dados

Use isto para definir o que pode ser escrito em algumas células, para que uma entrada errada seja impedida ou questionada.

1. Selecione as células.
2. Escolha **Validação de Dados** no grupo **Ferramentas de Dados** do separador **Dados**.
3. No separador **Definições**, escolha o que **Permitir**: **Qualquer valor** (sem regra), **Número inteiro**, **Decimal**, **Lista**, **Data**, **Hora**, **Comprimento do texto** ou **Personalizada**.
4. Para um número, data, hora ou comprimento de texto, escolha **Dados** (**entre**, **não está entre**, **igual a**, e assim por diante) e escreva os limites. Um limite pode ser um valor ou uma fórmula que começa por =.
5. Para uma **Lista**, escreva as opções separadas por vírgulas, como Vermelho,Verde,Azul, ou escreva um intervalo de células depois de =, como =$A$1:$A$5. A célula passa a mostrar uma seta com as opções. Desative **Lista pendente na célula** para ocultar a seta.
6. Para **Personalizada**, escreva uma fórmula que começa por =. A entrada é permitida quando a fórmula for verdadeira.
7. **Ignorar em branco** deixa passar uma célula vazia.

No separador **Mensagem de Entrada**, marque a caixa e escreva um título e uma mensagem. Aparecem ao lado da célula sempre que ela estiver selecionada.

No separador **Alerta de Erro**, escolha o que acontece a uma entrada não permitida. **Parar** recusa-a. **Aviso** pergunta se a deve manter. **Informação** avisa quem escreve e mantém a entrada. Escreva um título e uma mensagem para o alerta.

**Assinalar Dados Inválidos** (ao lado de **Validação de Dados**) coloca um círculo vermelho em cada célula da folha cujo valor a sua regra não permite, como um número que já lá estava antes da regra, ou o resultado de uma fórmula. Uma mensagem indica se todas as células estão corretas. Uma célula que corrija perde o seu círculo. **Limpar Assinalações de Validação** retira todos os círculos.

**Limpar Tudo** retira as regras das células selecionadas. A nova regra substitui qualquer regra que as células selecionadas tivessem; as outras células mantêm as suas. Alterar regras é um passo que pode anular, e as regras são guardadas no ficheiro.

## Consolidar

Use isto para combinar intervalos semelhantes, como a mesma tabela de várias folhas, numa só tabela.

1. Escolha **Consolidar** no grupo **Ferramentas de Dados** do separador **Dados**.
2. Escolha a **Função** a usar na combinação, como **Soma** ou **Média**.
3. Escreva um intervalo na caixa e prima **Adicionar**. Use **Folha2!A1:C10** para um intervalo noutra folha. Adicione cada intervalo que quer combinar. Selecione um intervalo na lista e prima **Eliminar** para o retirar.
4. Se os intervalos tiverem nomes na primeira linha, marque **A linha superior contém os nomes das colunas**. Se tiverem nomes na primeira coluna, marque **A coluna da esquerda contém os nomes das linhas**.
5. Em **Colocar o resultado em**, escreva a primeira célula do resultado.
6. Marque **Criar ligações aos dados de origem** se cada resultado deve ser uma fórmula que lê as células de origem, para mudar quando elas mudarem. Sem esta opção, os resultados são números simples.
7. Prima **OK**.

Sem nomes marcados, combina-se a célula na mesma posição em cada intervalo (a primeira célula de cada intervalo, depois a segunda, e assim por diante). Com os nomes marcados, combinam-se as células com os mesmos nomes, onde quer que estejam em cada intervalo, e um nome que só alguns intervalos têm continua a receber uma linha ou coluna. **Consolidar** é um passo que pode anular.

## Folha de Previsão

A Folha de Previsão cria uma nova folha com uma tabela e um gráfico de valores futuros, calculados a partir de uma linha temporal e de uma série de valores. Encontra a tendência e a estação repetitiva dos valores, e acrescenta uma faixa dentro da qual os valores futuros provavelmente se mantêm.

1. Selecione a linha temporal e os valores, duas colunas lado a lado (a linha temporal primeiro). A linha temporal são datas ou números com a mesma distância entre si, como cada mês ou de 5 em 5 unidades. Uma primeira linha de nomes fica de fora.
2. Escolha **Folha de Previsão** no grupo **Previsão** do separador **Dados**.
3. Uma imagem mostra a previsão e muda à medida que altera as opções. Escolha **Gráfico de linhas** ou **Gráfico de colunas**.
4. Defina as opções:
   - **Intervalo da linha temporal** e **Intervalo de valores** são as células a ler. Escreva aqui outras células se a seleção estiver errada.
   - **Início da previsão** fica vazio para começar depois do último ponto. Uma data ou número anterior ao último ponto começa aí e usa apenas os pontos anteriores, para poder comparar a previsão com o que aconteceu.
   - **Fim da previsão** é a última data a prever. São previstos no máximo 10 000 pontos.
   - **Intervalo de confiança** indica o grau de certeza da faixa, começando em 95%.
   - **Sazonalidade** é encontrada ao analisar os valores, ou pode definir o comprimento da estação em pontos, ou escolher nenhuma. Os valores têm de conter duas estações completas.
   - **Preencher pontos em falta com** indica o que recebe um momento sem valor: uma linha reta entre os vizinhos, ou zero.
   - **Agregar duplicados utilizando** indica como se combinam os valores no mesmo momento: Média, Contar, CONTARA, Máx., Mediana, Mín. ou Soma.
   - **Incluir estatísticas de previsão** acrescenta uma tabela com Alfa, Beta, Gama, MASE, SMAPE, MAE e RMSE, que indicam o ajuste da previsão aos valores.
5. Se algo estiver errado, a caixa diz o quê em linguagem simples (passos que não são iguais, pontos insuficientes, texto nos valores), e **Criar** fica desativado.
6. Prima **Criar**.

A nova folha é colocada antes da que estava a usar. A sua tabela tem as colunas Linha temporal, Valores, Previsão, Limite inferior de confiança e Limite superior de confiança. A Previsão e os limites são fórmulas que usam `FORECAST.ETS` e `FORECAST.ETS.CONFINT` sobre as colunas Linha temporal e Valores da própria tabela, por isso são recalculadas quando altera um valor. O gráfico mostra os valores, a previsão e a faixa. A Folha de Previsão é um passo que pode anular.

Os números podem diferir em alguns por cento dos de outros programas de folhas de cálculo, que usam o seu próprio método para ajustar a suavização, e uma previsão que um ficheiro já contém mantém os valores que o ficheiro guardou, até os dados que ela lê serem alterados.

Um ficheiro com mais de 1 000 000 de pontos não é utilizado.
