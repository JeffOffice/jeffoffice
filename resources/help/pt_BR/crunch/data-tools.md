# Ferramentas de dados
<!-- keywords: previsão de planilha, previsão, valores futuros, intervalo de confiança, sazonalidade, tendência, suavização, formulário, formulário de dados, entrada de dados, registro, registros, novo registro, localizar próximo, localizar anterior, critérios, gerenciador de cenários, cenários, e se, o que acontece se, resumo de cenário, dados, guia dados, texto para colunas, dividir, separador, vírgula, tabulação, largura fixa, remover duplicatas, atingir meta, subtotal, totais, filtro avançado, condições, validação de dados, circular dados inválidos, oval vermelho, limpar círculos de validação, consolidar, combinar intervalos, permitir, lista, lista suspensa, mensagem de entrada, alerta de erro, linhas repetidas, único, arquivo de texto, csv, de texto, colunas, delimitador, aspas, editar links, atualizar valores, alterar origem, links, externo, origem, quebrar vínculo, verificar status, abrir origem -->

A guia **Dados** tem as ferramentas que trabalham com uma lista de valores: obter texto de um arquivo, colocar uma lista em ordem, dividir texto em colunas e eliminar linhas repetidas.

## De Texto/CSV

**De Texto/CSV** abre um arquivo de texto (.csv, .tsv ou .txt). É o mesmo que **Abrir Arquivo de Texto** na guia Arquivo. O JeffCrunch pergunta como o arquivo está dividido em colunas e que tipo de texto ele contém, e depois o mostra como uma nova pasta de trabalho.

## Editar Links

Se uma pasta de trabalho tem fórmulas que leem outra pasta de trabalho (como ='[Preços.xlsx]Planilha1'!A1), **Editar Links**, no grupo **Obter Dados**, lista essas outras pastas de trabalho, chamadas origens. Cada linha mostra onde está a origem e o seu status. Até que a pasta de trabalho seja aberta novamente com as suas origens, ela mostra os números que o arquivo armazenou a partir delas.

- **Verificar Status** procura cada origem neste computador e diz **Encontrada**, ou diz que ela não foi encontrada. Uma origem na web não é baixada, por isso o status diz que ela não foi verificada.
- **Atualizar Valores** lê a origem escolhida deste computador e dá a cada célula que é apenas uma referência a uma de suas células o valor que essa célula tem agora. Uma célula que faz mais com o valor, como `=[1]Preços!A1*2`, mantém o número que o arquivo armazenou, e a mensagem abaixo da lista diz quantas células foram atualizadas e quantas não foram. Atualizar Valores é uma única etapa, por isso **Desfazer** restaura os valores antigos. Os novos valores também são gravados na parte do próprio vínculo no arquivo quando você salva, de modo que a pasta de trabalho os mostra na próxima vez que for aberta, mesmo que a origem não esteja disponível.
- **Alterar Origem** permite escolher outro arquivo para o vínculo selecionado, por exemplo quando a pasta de trabalho foi movida. As fórmulas permanecem como estão. O novo local é gravado no arquivo quando você salva, e **Desfazer** desfaz a alteração.
- **Abrir Origem** abre a origem escolhida em uma nova janela, quando ela puder ser encontrada.
- **Quebrar Vínculo** transforma cada célula que lê a origem escolhida no valor que ela mostra agora, e um nome que representava um local nela em **#REF!**. O JeffCrunch pergunta antes. Quebrar Vínculo é uma única etapa, por isso **Desfazer** restaura as fórmulas, os nomes e os links, até que você salve a pasta de trabalho. Quando não sobra nenhum link, o arquivo é salvo sem as partes que os continham.

## Formulário

**Formulário** (no grupo **Ferramentas de Dados** e na caixa de pesquisa) mostra uma linha de uma lista por vez, como um pequeno formulário. Uma lista é uma tabela, ou um intervalo de células com uma linha de nomes no topo e uma linha de dados abaixo de cada nome. Clique primeiro em qualquer célula da lista.

- Cada caixa é uma coluna, com o nome da linha superior como rótulo. Digite em uma caixa para alterar o valor. Uma caixa em cinza contém uma fórmula: o formulário mostra o resultado, e você altera a fórmula na planilha.
- **Localizar Próximo** e **Localizar Anterior** vão para a linha seguinte e para a linha anterior. Ir para outra linha, ou pressionar **Enter**, grava o que você digitou nesta linha.
- **Novo** mostra um formulário vazio. Preencha-o e, em seguida, pressione **Novo** de novo, **Localizar Próximo** ou **Fechar**. A linha é adicionada abaixo da última linha da lista. Uma tabela cresce uma linha. Se uma célula abaixo da lista não estiver vazia, o formulário avisa e não adiciona nada.
- **Excluir** remove a linha que você está vendo, depois de pedir confirmação. As linhas abaixo dela sobem. Uma tabela sempre mantém pelo menos uma linha de dados. Uma linha oculta, ou que um filtro oculta, não é mostrada no formulário, não é contada e nunca é excluída por ele. Se a lista for longa demais para ser percorrida inteira, o formulário informa que mostra apenas as primeiras linhas.
- **Restaurar** desfaz o que você digitou nesta linha, antes de sair dela.
- **Critérios** permite procurar linhas. Digite uma condição em uma ou mais caixas e pressione **Localizar Próximo** ou **Localizar Anterior**. Uma condição é um valor, como **Ana** (o texto corresponde ao início da palavra, e **?** e ***** representam letras), ou um valor precedido de um sinal: **>100**, **<=5**, **<>0**, **=Oslo**. Uma linha deve atender a todas as condições digitadas. Pressione **Formulário** para voltar, ou **Limpar** para recomeçar. Enquanto houver critérios definidos, **Localizar Próximo** e **Localizar Anterior** pulam as linhas que não os atendem.
- **Fechar** grava a linha e fecha o formulário.

Cada linha que você altera, adiciona ou exclui é uma etapa, por isso **Desfazer** na planilha a desfaz de uma só vez.

## Classificar e filtrar

O grupo **Classificar e Filtrar** tem os mesmos comandos do menu **Classificar e Filtrar** da guia Página Inicial. Veja [Classificar e filtrar](sort-filter).

## Texto para Colunas

Use isto quando uma coluna contém vários trechos de texto que você quer em colunas separadas, como "Silva, Ana, 31".

1. Selecione as células de **uma** coluna.
2. Escolha **Texto para Colunas** na guia **Dados**.
3. Escolha como dividir o texto:
   - **Dividir em um separador**: marque os sinais que ficam entre os trechos: **Tabulação**, **Ponto e vírgula**, **Vírgula**, **Espaço** ou **Outro**, em que você digita o seu próprio sinal. **Tratar separadores consecutivos como um só** impede que duas vírgulas seguidas criem um trecho vazio. **Texto entre aspas** indica o sinal que envolve um trecho que não deve ser dividido, como "Lee, Bo". Dentro das aspas, dois sinais seguidos representam um só.
   - **Dividir em posições fixas**: digite as posições em que uma nova coluna começa, contando caracteres a partir de 1. Por exemplo, 5, 12 inicia uma nova coluna no quinto e no décimo segundo caractere.
4. Escolha o que cada trecho é: **Geral** lê números e datas como se você os tivesse digitado, e **Texto** mantém cada trecho como está, de modo que "007" continua "007".
5. Em **Colocar as partes a partir de**, digite a primeira célula para as partes. Ela começa como a primeira célula que você selecionou.
6. A caixa abaixo das opções mostra as primeiras linhas divididas da maneira escolhida. Pressione **OK**.

As partes são colocadas nas células à direita da primeira célula, e substituem o que havia nelas. Texto para Colunas é uma etapa que você pode desfazer.

## Remover Duplicatas

Use isto para manter apenas a primeira de cada grupo de linhas repetidas.

1. Selecione as células da lista, ou uma célula dentro dela. O JeffCrunch usa todo o bloco de dados ao redor de uma única célula.
2. Escolha **Remover Duplicatas**.
3. Marque **Meus dados têm linha de cabeçalho** se a primeira linha contiver nomes. Essa linha permanece e não é comparada.
4. Marque as colunas que devem ser iguais para que duas linhas sejam consideradas repetidas. Duas linhas se repetem somente quando todas as colunas marcadas são iguais. Letras maiúsculas e minúsculas são consideradas iguais, e uma fórmula é considerada pelo valor que produz.
5. Pressione **OK**. A caixa de diálogo informa quantas linhas foram removidas e quantas restam.

As linhas abaixo de uma linha removida sobem dentro das células selecionadas, com as suas formatações e fórmulas. As células abaixo da seleção não se movem. As linhas vazias deixadas no final da seleção são limpas. Remover Duplicatas é uma etapa que você pode desfazer.

## Atingir Meta

Use isto quando você sabe o resultado que quer, mas não o número que o produz. Por exemplo, uma célula de pagamento contém uma fórmula que usa uma taxa de juros, e você quer que o pagamento seja 500. O Atingir Meta testa números na célula da taxa até que o pagamento seja 500.

1. Escolha **Atingir Meta** na guia **Dados**.
2. Em **Célula com a fórmula**, digite a célula que contém a fórmula, como B3. Ela começa como a célula que você selecionou.
3. Em **Valor desejado**, digite o resultado que você quer que a fórmula produza.
4. Em **Célula a alterar**, digite a célula que o Atingir Meta pode alterar, como B1. Ela deve conter um número, não uma fórmula.
5. Pressione **OK**.

Se o Atingir Meta encontrar um número, ele permanece na célula a alterar, e a barra de status informa qual é. Se não conseguir, nada muda e você é avisado. Tente outro número inicial na célula a alterar e confira se a fórmula realmente usa essa célula. Atingir Meta é uma etapa que você pode desfazer.

## Gerenciador de Cenários

Um cenário é um conjunto nomeado de valores para algumas células. Use cenários para testar hipóteses do tipo "e se" em uma planilha. Por exemplo, mantenha um cenário "Vendas baixas" e outro "Vendas altas" para as mesmas células de entrada, e mostre aquele que você quer analisar.

1. Escolha **Gerenciador de Cenários** na guia **Dados**.
2. Pressione **Adicionar**. Digite um nome, como Vendas altas. Em **Células alteradas**, digite as células que o cenário define, como B1:B3 ou D5. Um cenário pode alterar até 32 células. As células que você selecionou são preenchidas automaticamente. Cada célula começa com o valor que tem agora. Digite na tabela o valor que este cenário deve usar e pressione **OK**.
3. Escolha um cenário na lista e pressione **Mostrar** para colocar os seus valores nas células. Clicar duas vezes em um nome faz o mesmo. Pressione **Editar** para alterar o nome, o comentário ou os valores. Pressione **Excluir** para removê-lo.

Mostrar, Adicionar, Editar e Excluir são, cada um, uma etapa que você pode desfazer.

**Resumo** coloca todos os cenários lado a lado em uma nova planilha chamada Resumo de Cenário. Ele pede as células de resultado, que são as células que você quer comparar, como o total em B10. Para cada cenário, o resumo mostra os valores das células alteradas e o resultado das células de resultado, calculado com os valores daquele cenário. Uma coluna chamada Valores Atuais mostra como estavam as células quando você fez o resumo. Suas próprias células são deixadas como estavam.

Os cenários são salvos na pasta de trabalho. Uma pasta de trabalho feita por outro programa mantém os seus cenários, e você os vê aqui. Uma cópia salva como arquivo .xls mais antigo ou como .ods não os contém, e o salvamento avisa você disso.

## Subtotal

Use isto para colocar uma linha de totais abaixo de cada grupo de linhas que têm a mesma entrada em uma coluna, e um total geral no final. A lista precisa de uma linha de cabeçalho e deve estar classificada pela coluna pela qual você agrupa, para que entradas iguais fiquem uma ao lado da outra.

1. Selecione uma célula na lista.
2. Escolha **Subtotal** no grupo **Estrutura de Tópicos** da guia **Dados**.
3. Em **A cada alteração em**, escolha a coluna que agrupa as linhas, como Região.
4. Em **Usar função**, escolha o que calcular: **Soma**, **Contagem**, **Média**, **Máx** e **Mín**, entre outras.
5. Em **Adicionar subtotal a**, marque cada coluna a totalizar.
6. Deixe **Substituir subtotais atuais** marcado para trocar os subtotais que já existem. Pressione **OK**.

Cada linha de total diz "Total Leste" (ou a entrada a que se refere) e contém uma fórmula SUBTOTAL, por isso ela ignora as outras linhas de total. As linhas de cada grupo ficam agrupadas, de modo que você pode usar os botões 1, 2 e 3 ao lado das linhas para mostrar somente os totais. Para remover os totais, abra **Subtotal** de novo e pressione **Remover Tudo**. Adicionar ou remover subtotais é uma etapa que você pode desfazer.

## Filtro Avançado

Os botões de filtro escolhem linhas por uma coluna de cada vez. **Avançado** (no grupo **Classificar e Filtrar**) permite escrever as condições na planilha, de modo que elas podem ser tão longas quanto for preciso.

1. Ao lado da lista, copie os cabeçalhos das colunas que você quer testar para uma linha de células vazias.
2. Sob cada cabeçalho, escreva uma condição. As condições na **mesma linha** devem ser todas verdadeiras (isto é "e"). As condições em **linhas diferentes** são alternativas: uma linha da lista precisa atender a apenas uma dessas linhas (isto é "ou").
3. Escolha **Avançado**. Confira se **Intervalo da lista** é a lista com a sua linha de cabeçalho, e digite as células dos seus cabeçalhos e condições em **Intervalo de condições**.
4. Escolha **Ocultar as outras linhas, na lista**, ou **Copiar as linhas para outro local** e digite a primeira célula do local, como H1. Marque **Omitir linhas que repetem uma anterior** para copiar apenas uma de cada.
5. Pressione **OK**.

Como escrever uma condição:

- Uma palavra, como **An**, escolhe textos que começam com ela. Letras maiúsculas e minúsculas são consideradas iguais. **?** representa qualquer caractere e ***** qualquer sequência de caracteres.
- Um número escolhe células iguais a ele.
- **=Ana** escolhe textos exatamente iguais a Ana. **=** sozinho escolhe células vazias.
- **>**, **>=**, **<**, **<=** e **<>** (diferente) antes de um número ou de uma palavra fazem a comparação, como **>25**. **<>** sozinho escolhe células que não estão vazias.

Uma linha de condições sem nada nela escolhe todas as linhas. Para mostrar de novo as linhas ocultas, escolha **Limpar**. Uma cópia é feita imediatamente e não muda quando a lista muda. Filtro Avançado é uma etapa que você pode desfazer.

## Validação de Dados

Use isto para definir o que pode ser digitado em algumas células, de modo que uma entrada errada seja bloqueada ou questionada.

1. Selecione as células.
2. Escolha **Validação de Dados** no grupo **Ferramentas de Dados** da guia **Dados**.
3. Na guia **Configurações**, escolha o que **Permitir**: **Qualquer valor** (sem regra), **Número inteiro**, **Decimal**, **Lista**, **Data**, **Hora**, **Comprimento do texto** ou **Personalizado**.
4. Para um número, data, hora ou comprimento de texto, escolha **Dados** (**entre**, **não entre**, **igual a** e assim por diante) e digite os limites. Um limite pode ser um valor ou uma fórmula que começa com =.
5. Para uma **Lista**, digite as opções separadas por vírgulas, como Vermelho,Verde,Azul, ou digite um intervalo de células após =, como =$A$1:$A$5. A célula então mostra uma seta com as opções. Desmarque **Lista suspensa na célula** para ocultar a seta.
6. Para **Personalizado**, digite uma fórmula que começa com =. A entrada é permitida quando a fórmula for verdadeira.
7. **Ignorar vazio** permite que uma célula vazia passe.

Na guia **Mensagem de Entrada**, marque a caixa e escreva um título e uma mensagem. Eles aparecem ao lado da célula sempre que ela estiver selecionada.

Na guia **Alerta de Erro**, escolha o que acontece com uma entrada não permitida. **Parar** a recusa. **Aviso** pergunta se você quer mantê-la. **Informações** avisa a pessoa e mantém a entrada. Escreva um título e uma mensagem para o alerta.

**Circular Dados Inválidos** (ao lado de **Validação de Dados**) coloca um oval vermelho em volta de cada célula da planilha cujo valor a sua regra não permite, como um número que já estava lá antes da regra, ou o resultado de uma fórmula. Uma mensagem informa se todas as células estão corretas. Uma célula que você corrige perde o seu oval. **Limpar Círculos de Validação** remove todos os ovais.

**Limpar Tudo** remove as regras das células selecionadas. A nova regra substitui qualquer regra que as células selecionadas tivessem; outras células mantêm as suas. Alterar regras é uma etapa que você pode desfazer, e as regras são salvas no arquivo.

## Consolidar

Use isto para combinar intervalos semelhantes, como a mesma tabela de várias planilhas, em uma única tabela.

1. Escolha **Consolidar** no grupo **Ferramentas de Dados** da guia **Dados**.
2. Escolha a **Função** com que combinar, como **Soma** ou **Média**.
3. Digite um intervalo na caixa e pressione **Adicionar**. Use **Planilha2!A1:C10** para um intervalo em outra planilha. Adicione cada intervalo que quer combinar. Selecione um intervalo na lista e pressione **Excluir** para removê-lo.
4. Se os intervalos tiverem nomes na primeira linha, marque **A linha superior contém os nomes das colunas**. Se tiverem nomes na primeira coluna, marque **A coluna da esquerda contém os nomes das linhas**.
5. Em **Colocar o resultado em**, digite a primeira célula do resultado.
6. Marque **Criar vínculos com os dados de origem** se cada resultado deve ser uma fórmula que lê as suas células de origem, de modo que mude quando elas mudarem. Sem essa opção, os resultados são números simples.
7. Pressione **OK**.

Sem nenhum nome marcado, combina-se a célula na mesma posição de cada intervalo (a primeira célula de todos os intervalos, depois a segunda, e assim por diante). Com os nomes marcados, combinam-se as células com os mesmos nomes, onde quer que estejam em cada intervalo, e um nome que só alguns intervalos têm ainda recebe uma linha ou coluna. Consolidar é uma etapa que você pode desfazer.

## Planilha de Previsão

A Planilha de Previsão cria uma nova planilha com uma tabela e um gráfico de valores futuros, calculados a partir de uma linha do tempo e de uma série de valores. Ela identifica a tendência e a sazonalidade que se repete nos valores, e acrescenta uma faixa dentro da qual os valores futuros provavelmente ficarão.

1. Selecione a linha do tempo e os valores, duas colunas lado a lado (a linha do tempo primeiro). A linha do tempo são datas ou números com a mesma distância entre si, como todo mês ou a cada 5 unidades. Uma primeira linha de nomes não é considerada.
2. Escolha **Planilha de Previsão** no grupo **Previsão** da guia **Dados**.
3. Uma imagem mostra a previsão e muda conforme você altera as opções. Escolha **Gráfico de linhas** ou **Gráfico de colunas**.
4. Defina as opções:
   - **Intervalo da linha do tempo** e **Intervalo de valores** são as células a ler. Digite outras células aqui se a seleção estiver errada.
   - **Início da previsão** fica vazio para começar após o último ponto. Uma data ou número anterior ao último ponto começa ali e usa apenas os pontos anteriores a ele, para que você possa comparar a previsão com o que aconteceu.
   - **Fim da previsão** é o último momento a prever. No máximo 10.000 pontos são previstos.
   - **Intervalo de confiança** indica o grau de certeza da faixa, 95% no início.
   - **Sazonalidade** é identificada pela análise dos valores, ou você pode definir a duração da estação em pontos, ou escolher nenhuma. Os valores devem conter duas estações completas.
   - **Preencher pontos ausentes usando** define o que recebe um momento sem valor: uma linha reta entre os vizinhos, ou zero.
   - **Agregar duplicatas usando** define como os valores no mesmo momento são combinados: Média, Contagem, CONT.VALORES, Máx, Mediana, Mín ou Soma.
   - **Incluir estatísticas da previsão** acrescenta uma tabela com Alfa, Beta, Gama, MASE, SMAPE, MAE e RMSE, que indicam o quanto a previsão se ajusta aos valores.
5. Se algo estiver errado, a caixa de diálogo explica o problema em palavras simples (etapas que não são iguais, poucos pontos, texto nos valores), e **Criar** fica indisponível.
6. Pressione **Criar**.

A nova planilha é colocada antes da que você estava usando. Sua tabela tem as colunas Linha do tempo, Valores, Previsão, Limite inferior de confiança e Limite superior de confiança. A Previsão e os limites são fórmulas que usam `PREVISÃO.ETS` e `PREVISÃO.ETS.CONFINT` sobre as próprias colunas Linha do tempo e Valores da tabela, por isso são recalculadas quando você altera um valor. O gráfico mostra os valores, a previsão e a faixa. Planilha de Previsão é uma etapa que você pode desfazer.

Os números podem diferir em alguns por cento dos de outros programas de planilha, que usam sua própria forma de ajustar a suavização, e uma previsão que um arquivo já contém mantém os valores que o arquivo armazenou até que os dados que ela lê sejam alterados.

Um arquivo com mais de 1.000.000 de pontos não é usado.
