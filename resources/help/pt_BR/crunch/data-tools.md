# Ferramentas de dados
<!-- keywords: planilha de previsão, previsão, valores futuros, intervalo de confiança, sazonalidade, tendência, suavização, formulário, formulário de dados, entrada de dados, registro, registros, novo registro, localizar próximo, localizar anterior, critérios, gerenciador de cenários, cenários, e se, o que acontece se, resumo de cenário, dados, guia dados, texto para colunas, dividir, separador, vírgula, tabulação, largura fixa, remover duplicatas, atingir meta, subtotal, totais, filtro avançado, condições, validação de dados, circular dados inválidos, oval vermelho, limpar círculos de validação, consolidar, combinar intervalos, permitir, lista, suspensa, mensagem de entrada, alerta de erro, linhas repetidas, exclusivo, arquivo de texto, csv, de texto, colunas, delimitador, aspas, editar links, atualizar valores, alterar origem, links, externo, origem, quebrar vínculo, verificar status, abrir origem -->

A guia **Dados** tem as ferramentas que trabalham com uma lista de valores: obter texto de um arquivo, colocar uma lista em ordem, dividir um texto em colunas e tirar linhas repetidas.

## De Texto/CSV

**De Texto/CSV** abre um arquivo de texto (.csv, .tsv ou .txt). É o mesmo que **Abrir Arquivo de Texto** na guia Arquivo. O JeffCrunch pergunta como o arquivo é dividido em colunas e que tipo de texto ele contém, e então o mostra como uma nova pasta de trabalho.

## De Arquivo

**De Arquivo**, no grupo **Obter Dados**, carrega as linhas de um arquivo JSON, de um arquivo XML ou de outra pasta de trabalho como uma tabela. Só é possível carregar um arquivo deste computador. Um endereço da web ou um arquivo de outro computador é recusado.

1. Clique em **De Arquivo**, depois em **Procurar** e escolha o arquivo. As primeiras linhas aparecem de imediato, com o tipo que o JeffCrunch identificou para cada coluna a partir dessas primeiras linhas: número, data, verdadeiro ou falso, ou texto. O arquivo inteiro é verificado ao ser carregado, por isso uma coluna pode acabar como texto. Nada na planilha muda até você clicar em **Carregar**.
2. Quando o arquivo tem mais de uma lista, escolha qual carregar. Em um arquivo JSON, é uma lista de itens com valores nomeados, como `$.items`. Em um arquivo XML, é um item que se repete, como `/feed/entry`. Em uma pasta de trabalho, é uma planilha, uma tabela ou um intervalo nomeado.
3. Um item JSON dentro de outro item se torna uma coluna chamada `externo.interno`, até quatro níveis. Uma lista de valores simples é unida em uma única célula (os primeiros 10.000 valores) ou distribuída por colunas (as primeiras 16), conforme você escolher. Em XML, os atributos e os itens filhos se tornam colunas. Dois valores diferentes nunca compartilham uma coluna: quando dois receberiam o mesmo nome, o segundo recebe um número acrescentado ou, em XML, seu prefixo, e uma nota informa isso. Um nome repetido dentro de um mesmo item JSON mantém o último valor, e uma nota informa isso. Um número grande ou pequeno demais para uma célula é mantido como texto, como escrito, e um caractere que não pode ser lido aparece como um caractere de substituição; ambos os casos são indicados em nota.
4. Marque **Carregar todas as colunas como texto** para manter cada valor como está escrito. Uma coluna de números com zero à esquerda, como 007, sempre é mantida como texto.
5. Digite a célula onde a tabela deve ficar e clique em **Carregar**. Se a tabela cobrir células que contêm algo, o JeffCrunch pergunta antes de substituí-las. Se não houver espaço, ou se a planilha estiver protegida, você será avisado e nada muda.

A tabela é uma etapa de **Desfazer**. Ela guarda o arquivo e as suas escolhas. Para ler o arquivo novamente, clique em uma célula da tabela e clique em **Atualizar** na guia Dados. **Atualizar Tudo** é o botão grande ao lado: ele lê todas as tabelas criadas por Obter Dados e depois atualiza todas as tabelas dinâmicas, tudo como uma só etapa de Desfazer. A tabela aumenta ou diminui conforme as linhas do arquivo. As colunas que você adicionou ao lado da tabela permanecem: uma fórmula preenchida para baixo é preenchida nas novas linhas, e as linhas que saíram do arquivo também saem dela. Se o arquivo não existir mais ou não puder ser lido, você será avisado e nada muda. Se as colunas do próprio arquivo mudaram, você será informado quais estão ausentes ou são novas, e nada muda. Crie a tabela novamente com Obter Dados, ou recoloque as colunas no arquivo.

O vínculo com o arquivo é mantido na tabela, em uma pasta de trabalho `.xlsx` salva, em uma parte que outros programas ignoram. O vínculo guarda o caminho completo do arquivo neste computador, por isso quem receber a pasta de trabalho pode ler esse caminho. Para removê-lo antes de compartilhar a pasta de trabalho, use **Inspecionar**, depois **Vínculos com arquivos de dados** e depois **Remover**. Uma cópia salva em outro tipo de arquivo não mantém o vínculo, e **Verificar Compatibilidade** informa isso. **Inspecionar** lista os vínculos em **Vínculos com arquivos de dados** e pode removê-los. As tabelas e as suas células permanecem.

Os arquivos são lidos com limites: um arquivo de até 64 MB, 200.000 linhas, 256 colunas e 32.767 caracteres em uma célula. XML com declaração de tipo de documento ou entidades é recusado. Qualquer coisa cortada por um limite é informada.

## Editar Links

Se uma pasta de trabalho tem fórmulas que leem outra pasta de trabalho (como ='[Preços.xlsx]Planilha1'!A1), **Editar Links**, no grupo **Obter Dados**, lista essas outras pastas de trabalho, chamadas origens. Cada linha mostra onde está a origem e o seu status. Até que a pasta de trabalho seja aberta novamente com as suas origens, ela mostra os números que o arquivo guardou a partir delas.

- **Verificar Status** procura cada origem neste computador e informa **Encontrada**, ou informa que ela não foi encontrada. Uma origem na web não é baixada, por isso o status diz que não foi verificada.
- **Atualizar Valores** lê a origem escolhida deste computador e dá a cada célula que é apenas uma referência a uma de suas células o valor que essa célula tem agora. Uma célula que faz algo mais com o valor, como `=[1]Preços!A1*2`, mantém o número que o arquivo guardou, e a mensagem abaixo da lista informa quantas células foram atualizadas e quantas não foram. Atualizar Valores é uma única etapa, por isso **Desfazer** devolve os valores antigos. Os novos valores também são gravados na parte do próprio vínculo no arquivo quando você salva, de modo que a pasta de trabalho os mostra na próxima vez que for aberta, mesmo que a origem não esteja disponível.
- **Alterar Origem** permite escolher outro arquivo para o vínculo selecionado, por exemplo quando a pasta de trabalho foi movida. As fórmulas permanecem como estão. O novo local é gravado no arquivo ao salvar, e **Desfazer** desfaz a alteração.
- **Abrir Origem** abre a origem escolhida em uma nova janela, quando ela pode ser encontrada.
- **Quebrar Vínculo** transforma cada célula que lê a origem escolhida no valor que ela mostra agora, e um nome que representava um local nela em **#REF!**. O JeffCrunch pergunta antes. Quebrar Vínculo é uma única etapa, por isso **Desfazer** traz de volta as fórmulas, os nomes e os vínculos, até que você salve a pasta de trabalho. Quando não resta nenhum vínculo, o arquivo é salvo sem as partes que os continham.

## Formulário

**Formulário** (no grupo **Ferramentas de Dados** e na caixa de pesquisa) mostra uma linha de uma lista por vez, como um pequeno formulário. Uma lista é uma tabela, ou um bloco de células com uma linha de nomes no topo e uma linha de dados abaixo de cada nome. Clique antes em qualquer célula da lista.

- Cada caixa é uma coluna, identificada pelo nome da linha superior. Digite em uma caixa para alterar o valor. Uma caixa esmaecida contém uma fórmula: o formulário mostra o resultado, e você altera a fórmula na planilha.
- **Localizar Próximo** e **Localizar Anterior** vão para a linha seguinte e para a anterior. Ao ir para outra linha, ou pressionar **Enter**, o que você digitou nesta linha é gravado.
- **Novo** mostra um formulário vazio. Preencha-o e depois pressione **Novo** novamente, **Localizar Próximo** ou **Fechar**. A linha é adicionada abaixo da última linha da lista. Uma tabela cresce uma linha. Se uma célula abaixo da lista não estiver vazia, o formulário avisa e não adiciona nada.
- **Excluir** retira a linha que você está vendo, depois de pedir confirmação. As linhas abaixo dela sobem. Uma tabela sempre mantém pelo menos uma linha de dados. Uma linha oculta, ou que um filtro oculta, não é mostrada no formulário, não é contada e nunca é excluída por ele. Se a lista for longa demais para ser percorrida por inteiro, o formulário informa que mostra apenas as primeiras linhas.
- **Restaurar** desfaz o que você digitou nesta linha, antes de sair dela.
- **Critérios** permite procurar linhas. Digite uma condição em uma ou mais caixas e depois pressione **Localizar Próximo** ou **Localizar Anterior**. Uma condição é um valor, como **Ana** (o texto que começa com a palavra; **?** e ***** representam letras), ou um valor precedido de um sinal: **>100**, **<=5**, **<>0**, **=Oslo**. Uma linha deve atender a todas as condições digitadas. Pressione **Formulário** para voltar, ou **Limpar** para recomeçar. Enquanto houver critérios definidos, **Localizar Próximo** e **Localizar Anterior** pulam as linhas que não os atendem.
- **Fechar** grava a linha e fecha o formulário.

Cada linha que você altera, adiciona ou exclui é uma etapa, por isso **Desfazer** na planilha a desfaz de uma vez.

## Classificar e filtrar

O grupo **Classificar e Filtrar** tem os mesmos comandos do menu **Classificar e Filtrar** na guia Página Inicial. Veja [Classificar e filtrar](sort-filter).

## Texto para Colunas

Use isto quando uma coluna contém vários pedaços de texto que você quer em colunas separadas, como "Silva, Ana, 31".

1. Selecione as células de **uma** coluna.
2. Escolha **Texto para Colunas** na guia **Dados**.
3. Escolha como dividir o texto:
   - **Dividir em um separador**: marque os caracteres que ficam entre os pedaços: **Tabulação**, **Ponto e vírgula**, **Vírgula**, **Espaço** ou **Outro**, em que você digita o seu próprio caractere. **Tratar separadores consecutivos como um só** evita que duas vírgulas seguidas criem um pedaço vazio. **Texto entre aspas** indica o caractere que envolve um pedaço que não deve ser dividido, como "Lee, Bo". Dentro das aspas, dois caracteres seguidos valem por um só.
   - **Dividir em posições fixas**: digite as posições em que começa uma nova coluna, contando os caracteres a partir de 1. Por exemplo, 5, 12 inicia uma nova coluna no quinto e no décimo segundo caractere.
4. Escolha como ler cada pedaço: **Geral** lê números e datas como se você os tivesse digitado, e **Texto** mantém cada pedaço como está, para que "007" continue "007".
   Para tratar um pedaço de modo diferente, digite o seu número em **Pedaço** e escolha como ele é lido: **Igual ao anterior**, **Como digitado**, **Texto**, uma data escrita como **mês, dia, ano**, **dia, mês, ano** ou **ano, mês, dia**, ou **Excluir**, que não coloca esse pedaço em lugar nenhum. O primeiro pedaço é o 1. Um pedaço de data pode ter o mês por extenso (**5 mar 2024**, **5 de março de 2024**), pode ser separado por /, -, ponto, vírgula ou espaços, e pode ser seguido de uma hora (**10:30**, **10:30:45** ou **10:30 PM**). Um pedaço que não é uma data na ordem escolhida (mês 13, dia que não existe, ano que não tem dois ou quatro dígitos, ou hora sem sentido) é mantido como texto, exatamente como foi digitado; nunca é lido em outra ordem.
5. Em **Colocar os pedaços a partir de**, digite a primeira célula para os pedaços. Ela começa como a primeira célula que você selecionou.
6. A caixa abaixo das opções mostra as primeiras linhas divididas da maneira escolhida. Pressione **OK**.

Os pedaços vão para as células à direita da primeira célula, e substituem o que estava nelas. Texto para Colunas é uma etapa que você pode desfazer.

## Remover Duplicatas

Use isto para manter apenas a primeira de cada grupo de linhas que se repetem.

1. Selecione as células da lista, ou uma célula dentro dela. O JeffCrunch usa todo o bloco de dados ao redor de uma única célula.
2. Escolha **Remover Duplicatas**.
3. Marque **Meus dados têm cabeçalhos** se a primeira linha contém nomes. Essa linha permanece e não é comparada.
4. Marque as colunas que devem ser iguais para que duas linhas sejam consideradas repetidas. Duas linhas se repetem somente quando todas as colunas marcadas são iguais. Letras maiúsculas e minúsculas são consideradas iguais, e uma fórmula conta como o valor que produz.
5. Pressione **OK**. A caixa informa quantas linhas foram removidas e quantas restaram.

As linhas abaixo de uma linha removida sobem dentro das células selecionadas, com as suas formatações e fórmulas. As células abaixo da seleção não se movem. As linhas vazias que sobram no fim da seleção são limpas. Remover Duplicatas é uma etapa que você pode desfazer.

## Atingir Meta

Use isto quando você sabe o resultado que quer, mas não o número que o produz. Por exemplo, uma célula de pagamento tem uma fórmula que usa uma taxa de juros, e você quer que o pagamento seja 500. O Atingir Meta testa números na célula da taxa até que o pagamento seja 500.

1. Escolha **Atingir Meta** na guia **Dados**.
2. Em **Célula com a fórmula**, digite a célula que contém a fórmula, como B3. Ela começa como a célula que você selecionou.
3. Em **Valor desejado**, digite o resultado que você quer que a fórmula produza.
4. Em **Célula a alterar**, digite a célula que o Atingir Meta pode alterar, como B1. Ela deve conter um número, não uma fórmula.
5. Pressione **OK**.

Se o Atingir Meta encontrar um número, ele permanece na célula a alterar, e a barra de status informa qual é. Se não conseguir, nada muda e você é avisado. Experimente outro número inicial na célula a alterar e confira se a fórmula realmente usa essa célula. Atingir Meta é uma etapa que você pode desfazer.

## Gerenciador de Cenários

Um cenário é um conjunto nomeado de valores para algumas células. Use cenários para testar "e se" em uma única planilha. Por exemplo, mantenha um cenário "Vendas baixas" e um cenário "Vendas altas" para as mesmas células de entrada, e mostre aquele que quer analisar.

1. Escolha **Gerenciador de Cenários** na guia **Dados**.
2. Pressione **Adicionar**. Digite um nome, como Vendas altas. Em **Células alteradas**, digite as células que o cenário define, como B1:B3 ou D5. Um cenário pode alterar até 32 células. As células que você selecionou são preenchidas para você. Cada célula começa com o valor que tem agora. Digite na tabela o valor que este cenário deve usar e pressione **OK**.
3. Escolha um cenário na lista e pressione **Mostrar** para colocar os seus valores nas células. Clicar duas vezes em um nome faz o mesmo. Pressione **Editar** para alterar o nome, o comentário ou os valores. Pressione **Excluir** para removê-lo.

Mostrar, Adicionar, Editar e Excluir são, cada um, uma etapa que você pode desfazer.

**Resumo** coloca todos os cenários lado a lado em uma nova planilha chamada Resumo de Cenário. Ele pede as células de resultado, que são as células que você quer comparar, como o total em B10. Para cada cenário, o resumo mostra os valores das células alteradas e o que as células de resultado produzem, calculado com os valores daquele cenário. Uma coluna chamada Valores Atuais mostra como estavam as células quando você fez o resumo. As suas próprias células permanecem como estavam.

Os cenários são salvos na pasta de trabalho. Uma pasta de trabalho feita em outro programa mantém os seus cenários, e você os vê aqui. Uma cópia salva como arquivo .xls mais antigo ou como .ods não os guarda, e o salvamento informa isso.

## Subtotal

Use isto para colocar uma linha de totais abaixo de cada grupo de linhas que têm a mesma entrada em uma coluna, e um total geral no fim. A lista precisa de uma linha de cabeçalho e deve estar classificada pela coluna que você agrupa, para que entradas iguais fiquem juntas.

1. Selecione uma célula da lista.
2. Escolha **Subtotal** no grupo **Estrutura de Tópicos** da guia **Dados**.
3. Em **A cada alteração em**, escolha a coluna que agrupa as linhas, como Região.
4. Em **Usar função**, escolha o que calcular: **Soma**, **Contar**, **Média**, **Máx**, **Mín** e outras.
5. Em **Adicionar subtotal a**, marque cada coluna a totalizar.
6. Deixe **Substituir subtotais atuais** marcado para trocar os subtotais que já existem.
7. **Resumo abaixo dos dados** começa como a estrutura de tópicos da planilha o define. Deixe-o marcado para pôr cada linha de total abaixo do seu grupo, ou desmarque-o para pôr cada linha de total acima do seu grupo, com o total geral no topo. Pressione **OK**.

Cada linha de total diz "Total Leste" (ou a entrada a que se refere) e contém uma fórmula SUBTOTAL, por isso ignora as outras linhas de total. As linhas de cada grupo são agrupadas, e você pode usar os botões 1, 2 e 3 ao lado das linhas para mostrar somente os totais. Para retirar os totais, abra **Subtotal** novamente e pressione **Remover Tudo**. Se os totais foram para o outro lado de uma planilha que já tinha os seus próprios grupos, a barra de status informa que esses grupos agora têm suas linhas de resumo naquele lado, e Remover Tudo devolve o lado ao que era antes do primeiro subtotal, a menos que você tenha mudado isso depois. Com os totais acima, os botões da estrutura ficam nas linhas de total, de modo que um botão de mais ou de menos ao lado de "Total Leste" mostra ou oculta as linhas Leste abaixo dele. Se você desmarcar **Substituir subtotais atuais**, os novos totais devem ficar do mesmo lado dos que já existem. Caso contrário, nada é alterado e uma mensagem informa isso. Adicionar ou remover subtotais é uma etapa que você pode desfazer.

## Filtro Avançado

Os botões de filtro escolhem linhas por uma coluna de cada vez. **Avançado** (no grupo **Classificar e Filtrar**) permite escrever as condições na planilha, para que sejam tão longas quanto você precisar.

1. Ao lado da lista, copie os cabeçalhos das colunas que quer testar para uma linha de células vazias.
2. Abaixo de cada cabeçalho, escreva uma condição. As condições na **mesma linha** devem ser todas verdadeiras (isto é "e"). As condições em **linhas diferentes** são alternativas: uma linha da lista precisa atender a apenas uma dessas linhas (isto é "ou").
3. Escolha **Avançado**. Confira se o **Intervalo da lista** é a lista com a sua linha de cabeçalho, e digite as células dos seus cabeçalhos e condições em **Intervalo de condições**.
4. Escolha **Ocultar as outras linhas, na lista**, ou **Copiar as linhas para outro local** e digite a primeira célula do local, como H1. Marque **Omitir linhas que repetem uma anterior** para copiar apenas uma de cada.
5. Pressione **OK**.

Como escrever uma condição:

- Uma palavra, como **An**, escolhe o texto que começa com ela. Letras maiúsculas e minúsculas são consideradas iguais. **?** representa qualquer caractere e ***** qualquer sequência de caracteres.
- Um número escolhe as células iguais a ele.
- **=Ana** escolhe o texto exatamente igual a Ana. **=** sozinho escolhe células vazias.
- **>**, **>=**, **<**, **<=** e **<>** (diferente) antes de um número ou de uma palavra fazem a comparação, como **>25**. **<>** sozinho escolhe células que não estão vazias.

Uma linha de condições sem nada escrito escolhe todas as linhas. Para mostrar novamente as linhas ocultas, escolha **Limpar**. Uma cópia é feita de imediato e não muda quando a lista muda. Filtro Avançado é uma etapa que você pode desfazer.

## Validação de Dados

Use isto para definir o que pode ser digitado em algumas células, de modo que uma entrada errada seja impedida ou questionada.

1. Selecione as células.
2. Escolha **Validação de Dados** no grupo **Ferramentas de Dados** da guia **Dados**.
3. Na guia **Configurações**, escolha o que **Permitir**: **Qualquer valor** (sem regra), **Número inteiro**, **Decimal**, **Lista**, **Data**, **Hora**, **Comprimento do texto** ou **Personalizado**.
4. Para um número, uma data, uma hora ou um comprimento de texto, escolha **Dados** (**entre**, **não está entre**, **igual a** e assim por diante) e digite os limites. Um limite pode ser um valor ou uma fórmula que começa com =.
5. Para uma **Lista**, digite as opções separadas por vírgulas, como Vermelho,Verde,Azul, ou digite um intervalo de células depois de =, como =$A$1:$A$5. A célula então mostra uma seta com as opções. Desmarque **Lista suspensa na célula** para ocultar a seta.
6. Para **Personalizado**, digite uma fórmula que começa com =. A entrada é permitida quando a fórmula é verdadeira.
7. **Ignorar vazio** permite que uma célula vazia passe.

Na guia **Mensagem de Entrada**, marque a caixa e escreva um título e uma mensagem. Eles aparecem ao lado da célula sempre que ela é selecionada.

Na guia **Alerta de Erro**, escolha o que acontece com uma entrada não permitida. **Parar** a recusa. **Aviso** pergunta se deve mantê-la. **Informações** informa a pessoa e mantém a entrada. Escreva um título e uma mensagem para o alerta.

**Circular Dados Inválidos** (ao lado de **Validação de Dados**) coloca um oval vermelho em volta de cada célula da planilha cujo valor a sua regra não permite, como um número que já estava lá antes da regra, ou o resultado de uma fórmula. Uma mensagem informa se todas as células estão corretas. Uma célula que você corrige perde o seu oval. **Limpar Círculos de Validação** remove todos os ovais.

**Limpar Tudo** retira as regras das células selecionadas. A nova regra substitui qualquer regra que as células selecionadas tivessem; as outras células mantêm as suas. Alterar regras é uma etapa que você pode desfazer, e as regras são salvas no arquivo.

## Consolidar

Use isto para combinar intervalos semelhantes, como a mesma tabela de várias planilhas, em uma única tabela.

1. Escolha **Consolidar** no grupo **Ferramentas de Dados** da guia **Dados**.
2. Escolha a **Função** com que combinar, como **Soma** ou **Média**.
3. Digite um intervalo na caixa e pressione **Adicionar**. Use **Planilha2!A1:C10** para um intervalo em outra planilha. Adicione cada intervalo que quer combinar. Selecione um intervalo na lista e pressione **Excluir** para retirá-lo.
4. Se os intervalos têm nomes na primeira linha, marque **A linha superior contém os nomes das colunas**. Se têm nomes na primeira coluna, marque **A coluna da esquerda contém os nomes das linhas**.
5. Em **Colocar o resultado em**, digite a primeira célula do resultado.
6. Marque **Criar vínculos com os dados de origem** se cada resultado deve ser uma fórmula que lê as suas células de origem, de modo que mude quando elas mudarem. Sem essa opção, os resultados são números simples.
7. Pressione **OK**.

Sem nomes marcados, a célula na mesma posição em cada intervalo é combinada (a primeira célula de cada intervalo, depois a segunda, e assim por diante). Com nomes marcados, as células com os mesmos nomes são combinadas, estejam onde estiverem em cada intervalo, e um nome que só alguns intervalos têm ainda recebe uma linha ou coluna. Consolidar é uma etapa que você pode desfazer.

## Planilha de Previsão

A Planilha de Previsão cria uma nova planilha com uma tabela e um gráfico de valores futuros, calculados a partir de uma linha do tempo e de uma série de valores. Ela encontra a tendência e a sazonalidade que se repete nos valores, e acrescenta uma faixa dentro da qual os valores futuros provavelmente ficarão.

1. Selecione a linha do tempo e os valores, duas colunas lado a lado (a linha do tempo primeiro). A linha do tempo são datas ou números igualmente espaçados, como todo mês ou a cada 5 unidades. Uma primeira linha de nomes é deixada de fora.
2. Escolha **Planilha de Previsão** no grupo **Previsão** da guia **Dados**.
3. Uma imagem mostra a previsão e muda conforme você altera as opções. Escolha **Gráfico de linhas** ou **Gráfico de colunas**.
4. Defina as opções:
   - **Intervalo da linha do tempo** e **Intervalo de valores** são as células a ler. Escreva aqui outras células se a seleção estiver errada.
   - **Início da previsão** fica vazio para começar depois do último ponto. Uma data ou número anterior ao último ponto começa ali e usa apenas os pontos anteriores a ele, para que você possa comparar a previsão com o que aconteceu.
   - **Fim da previsão** é o último momento a prever. No máximo 10.000 pontos são previstos.
   - **Intervalo de confiança** é o grau de certeza da faixa, 95% no início.
   - **Sazonalidade** é encontrada observando os valores, ou você pode definir a duração da temporada em pontos, ou escolher nenhuma. Os valores devem conter duas temporadas completas.
   - **Preencher pontos ausentes usando** define o que um momento sem valor recebe: uma linha reta entre os seus vizinhos, ou zero.
   - **Agregar duplicatas usando** define como os valores no mesmo momento são combinados: Média, Contar, ContarA, Máx, Mediana, Mín ou Soma.
   - **Incluir estatísticas da previsão** acrescenta uma tabela com Alfa, Beta, Gama, MASE, SMAPE, MAE e RMSE, que indicam o quanto a previsão se ajusta aos valores.
5. Se algo estiver errado, a caixa diz o quê em palavras simples (passos que não são iguais, poucos pontos, texto nos valores), e **Criar** fica indisponível.
6. Pressione **Criar**.

A nova planilha é colocada antes da que você estava. Sua tabela tem as colunas Linha do tempo, Valores, Previsão, Limite inferior de confiança e Limite superior de confiança. A Previsão e os limites são fórmulas que usam `PREVISÃO.ETS` e `PREVISÃO.ETS.CONFINT` sobre as próprias colunas Linha do tempo e Valores da tabela, por isso são recalculadas quando você altera um valor. O gráfico mostra os valores, a previsão e a faixa. Planilha de Previsão é uma etapa que você pode desfazer.

Os números podem diferir em alguns por cento dos de outros programas de planilha, que usam o seu próprio modo de ajustar a suavização, e uma previsão que um arquivo já contém mantém os valores que o arquivo guardou até que os dados que ela lê sejam alterados.

Um arquivo com mais de 1.000.000 de pontos não é usado.
