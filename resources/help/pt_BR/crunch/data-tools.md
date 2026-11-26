# Ferramentas de dados
<!-- keywords: planilha de previsão, previsão, valores futuros, intervalo de confiança, sazonalidade, tendência, suavização, formulário, formulário de dados, entrada de dados, registro, registros, novo registro, localizar próximo, localizar anterior, critérios, gerenciador de cenários, cenários, e se, o que acontece se, resumo de cenário, dados, guia dados, texto para colunas, dividir, separador, vírgula, tabulação, largura fixa, remover duplicatas, atingir meta, subtotal, totais, filtro avançado, condições, validação de dados, circular dados inválidos, oval vermelho, limpar círculos de validação, consolidar, combinar intervalos, permitir, lista, suspensa, mensagem de entrada, alerta de erro, linhas repetidas, exclusivo, arquivo de texto, csv, de texto, colunas, delimitador, aspas, editar links, atualizar valores, alterar origem, links, externo, origem, quebrar vínculo, verificar status, abrir origem -->

A guia **Dados** tem as ferramentas que trabalham com uma lista de valores: obter texto de um arquivo, colocar uma lista em ordem, dividir um texto em colunas e remover linhas repetidas.

## De Texto/CSV

**De Texto/CSV** abre um arquivo de texto (.csv, .tsv ou .txt). É o mesmo que **Abrir Arquivo de Texto** na guia Arquivo. O JeffCrunch pergunta como o arquivo está dividido em colunas e o que cada coluna contém, e então mostra o resultado como uma nova pasta de trabalho. A mesma janela aparece quando você abre um arquivo de texto ou usa **Colar Importação de Texto**.

A janela adivinha o que cada coluna contém com base nas primeiras 200 linhas do arquivo e mostra a adivinhação na caixa acima da coluna. Altere qualquer coluna antes de abrir o arquivo. Um tipo que você alterou nunca é substituído por uma adivinhação posterior, mesmo que depois você mude o separador. A adivinhação faz apenas três coisas com uma coluna: deixa a coluna **Como digitado**, que é como um arquivo é lido quando nada é adivinhado; torna a coluna **Texto** para protegê-la; ou transforma a coluna em datas em uma ordem que os valores comprovam. Estas são as regras:

- Uma coluna permanece **Como digitado** quando o Como digitado lê seus valores corretamente: números, verdadeiro e falso, horas, datas escritas com nomes de meses, números com sinal, como -5 e +3, palavras e números de versão, como 1.13.2. Alterá-los só pioraria as coisas.
- Uma coluna é de **data** somente quando todos os valores têm um ano de quatro dígitos (ou são uma data completa, como 2024-05-13) e são lidos como data em uma ordem e em nenhuma outra (mês, dia, ano; dia, mês, ano; ou ano, mês, dia). As datas são mostradas como as datas que você digita são mostradas. Quando mais de uma ordem serve para todos os valores, como 05/06/2024, a coluna permanece como **Texto**, a janela a lista, e você escolhe a ordem na caixa dela.
- Valores com três números e sem ano de quatro dígitos, como 1.13.2 ou 13.05.24, nunca são transformados em datas pela adivinhação. Se o Como digitado os manteria como texto, a coluna permanece Como digitado; se o Como digitado os transformaria em datas, a coluna fica **Texto**. Em ambos os casos, a janela lista uma coluna que poderia conter datas com anos de dois dígitos, para que você escolha uma ordem.
- Uma coluna fica **Texto** quando um valor tem um zero à frente de mais dígitos ou tem mais de 15 dígitos, quando um sinal de mais vem antes de sete ou mais dígitos (um número de telefone), ou quando um expoente, como 1E5, aparece junto de outros valores.
- Um valor que começa com um sinal de igual é deixado para a caixa **Ler fórmulas**. Quando a caixa está marcada, a coluna permanece Como digitado para que o valor seja lido como fórmula. Quando não está marcada, a coluna fica **Texto**.
- Marque **A primeira linha contém nomes de colunas** e a primeira linha não é usada na adivinhação. A janela a marca quando a primeira linha é texto sobre números ou datas.
- As colunas depois das 100 primeiras são importadas como texto.

Somente as primeiras 200 linhas são analisadas, então um valor mais abaixo pode ser uma surpresa. Em uma coluna de datas, um valor mais abaixo que não seja uma data na ordem escolhida é importado como texto, nunca como uma data errada. Em uma coluna de Texto, todo valor é importado exatamente como escrito. Em uma coluna Como digitado, um valor mais abaixo é lido da mesma forma que o Como digitado o leria em qualquer outro lugar.

**Colar Importação de Texto** insere o texto como se você o tivesse digitado, com os separadores decimal e de milhar configurados no seu computador. Por isso sua janela não tem caixas para esses separadores, e a adivinhação lê o texto da mesma forma que a colagem. A janela de um arquivo tem essas caixas.

## De Arquivo

**De Arquivo**, no grupo **Obter Dados**, carrega as linhas de um arquivo JSON, de um arquivo XML ou de outra pasta de trabalho como uma tabela. Só é possível carregar um arquivo deste computador. Um endereço da web ou um arquivo de outro computador é recusado.

1. Clique em **De Arquivo**, depois em **Procurar** e escolha o arquivo. As primeiras linhas aparecem de imediato, com o tipo que o JeffCrunch identificou para cada coluna a partir dessas linhas: número, data, verdadeiro ou falso, ou texto. O arquivo inteiro é verificado ao ser carregado, então uma coluna pode acabar como texto. Nada na planilha muda até que você clique em **Carregar**.
2. Quando o arquivo tem mais de uma lista, escolha a que será carregada. Em um arquivo JSON, é uma lista de itens com valores nomeados, como `$.items`. Em um arquivo XML, é um item que se repete, como `/feed/entry`. Em uma pasta de trabalho, é uma planilha, uma tabela ou um intervalo nomeado.
3. Um item JSON dentro de outro item vira uma coluna chamada `externo.interno`, até quatro níveis. Uma lista de valores simples é unida em uma única célula (os primeiros 10.000 valores), ou distribuída por colunas (as primeiras 16), conforme você escolher. Em XML, os atributos e os itens filhos viram colunas. Dois valores diferentes nunca dividem uma coluna: quando dois receberiam o mesmo nome, o segundo recebe um número acrescentado ou, em XML, seu prefixo, e uma nota informa isso. Um nome repetido dentro de um mesmo item JSON mantém seu último valor, e uma nota informa isso. Um número grande ou pequeno demais para uma célula é mantido como texto, como escrito, e um caractere que não pode ser lido é mostrado como um caractere de substituição; ambos os casos são registrados em nota.
4. Marque **Carregar todas as colunas como texto** para manter todos os valores como estão escritos. Uma coluna de números com zero à esquerda, como 007, é sempre mantida como texto.
5. Digite a célula onde a tabela será colocada e clique em **Carregar**. Se a tabela cobrir células que contenham algo, o JeffCrunch pergunta antes de substituí-las. Se não houver espaço, ou se a planilha estiver protegida, você é avisado e nada muda.

A tabela é uma etapa de **Desfazer**. Ela guarda o arquivo e suas escolhas. Para ler o arquivo novamente, clique em uma célula da tabela e clique em **Atualizar** na guia Dados. **Atualizar Tudo** é o botão grande ao lado: ele lê todas as tabelas criadas por Obter Dados e depois atualiza todas as tabelas dinâmicas, tudo como uma única etapa de Desfazer. A tabela cresce ou diminui conforme as linhas do arquivo. As colunas que você acrescentou ao lado da tabela permanecem: uma fórmula que você preencheu para baixo é preenchida nas novas linhas, e as linhas que saíram do arquivo também saem dela. Se o arquivo não existir mais ou não puder ser lido, você é avisado e nada muda. Se as colunas do próprio arquivo mudaram, você é informado de quais estão faltando ou são novas, e nada muda. Crie a tabela novamente com Obter Dados, ou restaure as colunas no arquivo.

O vínculo com o arquivo fica guardado na tabela, em uma pasta de trabalho `.xlsx` salva, em uma parte que outros programas ignoram. O vínculo contém o caminho completo do arquivo neste computador, então qualquer pessoa que receba a pasta de trabalho pode ler esse caminho. Para removê-lo antes de compartilhar a pasta de trabalho, use **Inspecionar**, depois **Vínculos com arquivos de dados** e depois **Remover**. Uma cópia salva em outro tipo de arquivo não mantém o vínculo, e **Verificar Compatibilidade** informa isso. **Inspecionar** lista os vínculos em **Vínculos com arquivos de dados** e pode removê-los. As tabelas e suas células permanecem.

Os arquivos são lidos com limites: um arquivo de até 64 MB, 200.000 linhas, 256 colunas e 32.767 caracteres em uma célula. XML com declaração de tipo de documento ou entidades é recusado. Tudo o que for cortado por um limite é informado.

## Editar Links

Se uma pasta de trabalho tiver fórmulas que leem outra pasta de trabalho (como ='[Preços.xlsx]Planilha1'!A1), **Editar Links**, no grupo **Obter Dados**, lista essas outras pastas de trabalho, que são chamadas de origens. Cada linha mostra onde está a origem e o seu status. Até que a pasta de trabalho seja aberta novamente com suas origens, ela mostra os números que o arquivo guardou delas.

- **Verificar Status** procura cada origem neste computador e informa **Encontrada**, ou informa que ela não foi encontrada. Uma origem na web não é buscada, então seu status informa que não foi verificada.
- **Atualizar Valores** lê a origem escolhida deste computador e dá a cada célula que seja apenas uma referência a uma de suas células o valor que essa célula tem agora. Uma célula que faz mais com o valor, como `=[1]Preços!A1*2`, mantém o número que o arquivo guardou, e a mensagem abaixo da lista informa quantas células foram atualizadas e quantas não foram. Atualizar Valores é uma única etapa, então **Desfazer** devolve os valores antigos. Os novos valores também são gravados na parte do próprio vínculo no arquivo ao salvar, então a pasta de trabalho os mostra na próxima vez que for aberta, mesmo quando a origem não estiver disponível.
- **Alterar Origem** permite escolher outro arquivo para o vínculo escolhido, por exemplo quando a pasta de trabalho foi movida. As fórmulas permanecem como estão. O novo local é gravado no arquivo ao salvar, e **Desfazer** desfaz a alteração.
- **Abrir Origem** abre a origem escolhida em uma nova janela, quando ela puder ser encontrada.
- **Quebrar Vínculo** transforma cada célula que lê a origem escolhida no valor que ela mostra agora, e um nome que representava um local nela em **#REF!**. O JeffCrunch pergunta antes. Quebrar Vínculo é uma única etapa, então **Desfazer** devolve as fórmulas, os nomes e os vínculos, até que você salve a pasta de trabalho. Quando não sobra nenhum vínculo, o arquivo é salvo sem as partes que os continham.

## Formulário

**Formulário** (no grupo **Ferramentas de Dados** e na caixa de pesquisa) mostra uma linha de cada vez de uma lista, como um pequeno formulário. Uma lista é uma tabela, ou um intervalo de células com uma linha de nomes no topo e uma linha de dados sob cada nome. Clique primeiro em qualquer célula da lista.

- Cada caixa é uma coluna, identificada pelo nome da linha superior. Digite em uma caixa para alterar o valor. Uma caixa em cinza contém uma fórmula: o formulário mostra o resultado, e você altera a fórmula na planilha.
- **Localizar Próximo** e **Localizar Anterior** vão para a linha seguinte e para a anterior. Ir para outra linha, ou pressionar **Enter**, grava o que você digitou nesta linha.
- **Novo** mostra um formulário vazio. Preencha-o e depois pressione **Novo** de novo, **Localizar Próximo** ou **Fechar**. A linha é acrescentada abaixo da última linha da lista. Uma tabela cresce uma linha. Se uma célula abaixo da lista não estiver vazia, o formulário avisa e não acrescenta nada.
- **Excluir** remove a linha que você está vendo, depois de pedir confirmação. As linhas abaixo dela sobem. Uma tabela sempre mantém pelo menos uma linha de dados. Uma linha oculta, ou que um filtro oculta, não é mostrada no formulário, não é contada e nunca é excluída por ele. Se a lista for longa demais para ser percorrida inteira, o formulário informa que mostra apenas as primeiras linhas.
- **Restaurar** desfaz o que você digitou nesta linha, antes de sair dela.
- **Critérios** permite procurar linhas. Digite uma condição em uma ou mais caixas e depois pressione **Localizar Próximo** ou **Localizar Anterior**. Uma condição é um valor, como **Ana** (o texto corresponde ao início da palavra, e **?** e ***** representam letras), ou um valor precedido de um sinal: **>100**, **<=5**, **<>0**, **=Oslo**. Uma linha deve atender a todas as condições que você digitar. Pressione **Formulário** para voltar, ou **Limpar** para recomeçar. Enquanto houver critérios definidos, **Localizar Próximo** e **Localizar Anterior** pulam as linhas que não os atendem.
- **Fechar** grava a linha e fecha o formulário.

Cada linha que você altera, acrescenta ou exclui é uma etapa, então **Desfazer** na planilha desfaz tudo de uma vez.

## Classificar e filtrar

O grupo **Classificar e Filtrar** tem os mesmos comandos do menu **Classificar e Filtrar** da guia Página Inicial. Veja [Classificar e filtrar](sort-filter).

## Texto para Colunas

Use isto quando uma coluna contém vários pedaços de texto que você quer em colunas separadas, como "Silva, Ana, 31".

1. Selecione as células de **uma** coluna.
2. Escolha **Texto para Colunas** na guia **Dados**.
3. Escolha como dividir o texto:
   - **Dividir em um separador**: marque os sinais que ficam entre os pedaços: **Tabulação**, **Ponto e vírgula**, **Vírgula**, **Espaço** ou **Outro**, em que você digita seu próprio sinal. **Tratar separadores consecutivos como um só** impede que duas vírgulas seguidas criem um pedaço vazio. **Texto entre aspas** indica o sinal que envolve um pedaço que não deve ser dividido, como "Lee, Bo". Dentro das aspas, dois sinais seguidos valem por um.
   - **Dividir em posições fixas**: digite as posições em que começa uma nova coluna, contando os caracteres a partir de 1. Por exemplo, 5, 12 inicia uma nova coluna no quinto e no décimo segundo caractere.
4. Escolha o que cada pedaço é: **Geral** lê números e datas como se você os tivesse digitado, e **Texto** mantém cada pedaço como está, para que "007" continue "007".
   Para tratar um pedaço de modo diferente, digite o número dele em **Pedaço** e escolha como ele é lido: **Mesmo que acima**, **Como digitado**, **Texto**, uma data escrita como **mês, dia, ano**, **dia, mês, ano** ou **ano, mês, dia**, ou **Ignorar**, que não coloca esse pedaço em lugar nenhum. O primeiro pedaço é o 1. Um pedaço de data pode ter o mês por extenso (**5 mar 2024**, **5 de março de 2024**) em inglês ou no seu idioma, pode ser separado por /, -, ponto, vírgula ou espaços, e pode ser seguido de uma hora (**10:30**, **10:30:45** ou **10:30 PM**). Um pedaço que não seja uma data na ordem escolhida (um mês 13, um dia que não existe, um ano que não tenha dois ou quatro dígitos, ou uma hora que não faça sentido) é mantido como texto, exatamente como foi digitado; nunca é lido em outra ordem.
5. Em **Colocar os pedaços a partir de**, digite a primeira célula para os pedaços. Ela começa como a primeira célula que você selecionou.
6. A caixa abaixo das escolhas mostra as primeiras linhas divididas da forma que você escolheu. Pressione **OK**.

Os pedaços vão para as células à direita da primeira célula, e substituem o que estava nelas. Texto para Colunas é uma etapa que você pode desfazer.

## Remover Duplicatas

Use isto para manter apenas a primeira de cada grupo de linhas que se repetem.

1. Selecione as células da lista, ou uma célula dentro dela. O JeffCrunch usa todo o bloco de dados em torno de uma única célula.
2. Escolha **Remover Duplicatas**.
3. Marque **Meus dados têm cabeçalho** se a primeira linha tiver nomes. Essa linha permanece e não é comparada.
4. Marque as colunas que devem ser iguais para que duas linhas sejam consideradas repetidas. Duas linhas se repetem somente quando todas as colunas marcadas são iguais. Letras maiúsculas e minúsculas são consideradas iguais, e uma fórmula conta pelo valor que produz.
5. Pressione **OK**. A caixa de diálogo informa quantas linhas foram removidas e quantas restaram.

As linhas abaixo de uma linha removida sobem dentro das células selecionadas, com suas formatações e fórmulas. As células abaixo da seleção não se movem. As linhas vazias deixadas no final da seleção são limpas. Remover Duplicatas é uma etapa que você pode desfazer.

## Atingir Meta

Use isto quando você sabe a resposta que quer, mas não o número que a produz. Por exemplo, uma célula de pagamento contém uma fórmula que usa uma taxa de juros, e você quer que o pagamento seja 500. Atingir Meta tenta números na célula da taxa até que o pagamento seja 500.

1. Escolha **Atingir Meta** na guia **Dados**.
2. Em **Célula com a fórmula**, digite a célula que contém a fórmula, como B3. Ela começa como a célula que você selecionou.
3. Em **Valor desejado**, digite a resposta que você quer que a fórmula produza.
4. Em **Célula a alterar**, digite a célula que Atingir Meta pode alterar, como B1. Ela deve conter um número, não uma fórmula.
5. Pressione **OK**.

Se Atingir Meta encontrar um número, ele permanece na célula a alterar, e a barra de status informa qual é. Se não encontrar, nada muda e você é avisado. Experimente um número inicial diferente na célula a alterar e confira se a fórmula realmente usa essa célula. Atingir Meta é uma etapa que você pode desfazer.

## Gerenciador de Cenários

Um cenário é um conjunto nomeado de valores para algumas células. Use cenários para testar "e se" em uma única planilha. Por exemplo, mantenha um cenário "Vendas baixas" e um cenário "Vendas altas" para as mesmas células de entrada, e mostre aquele que você quer analisar.

1. Escolha **Gerenciador de Cenários** na guia **Dados**.
2. Pressione **Adicionar**. Digite um nome, como Vendas altas. Em **Células alteradas**, digite as células que o cenário define, como B1:B3 ou D5. Um cenário pode alterar até 32 células. As células que você selecionou são preenchidas para você. Cada célula começa com o valor que tem agora. Digite na tabela o valor que este cenário deve usar e pressione **OK**.
3. Escolha um cenário na lista e pressione **Mostrar** para colocar seus valores nas células. Clicar duas vezes em um nome faz o mesmo. Pressione **Editar** para alterar seu nome, seu comentário ou seus valores. Pressione **Excluir** para removê-lo.

Mostrar, Adicionar, Editar e Excluir são cada uma uma etapa que você pode desfazer.

**Resumo** coloca todos os cenários lado a lado em uma nova planilha chamada Resumo de Cenário. Ele pede as células de resultado, que são as células que você quer comparar, como o total em B10. Para cada cenário, o resumo mostra os valores das células alteradas e o que as células de resultado produzem, calculado com os valores daquele cenário. Uma coluna chamada Valores Atuais mostra como estavam as células quando você fez o resumo. Suas próprias células permanecem como estavam.

Os cenários são salvos na pasta de trabalho. Uma pasta de trabalho feita em outro programa mantém seus cenários, e você os vê aqui. Uma cópia salva como arquivo .xls mais antigo ou como .ods não os mantém, e o salvamento informa isso.

## Subtotal

Use isto para colocar uma linha de totais sob cada grupo de linhas que têm a mesma entrada em uma coluna, e um total geral no final. A lista precisa de uma linha de cabeçalho e deve estar classificada pela coluna pela qual você agrupa, para que entradas iguais fiquem juntas.

1. Selecione uma célula da lista.
2. Escolha **Subtotal** no grupo **Estrutura de Tópicos** da guia **Dados**.
3. Em **A cada alteração em**, escolha a coluna que agrupa as linhas, como Região.
4. Em **Usar função**, escolha o que calcular: **Soma**, **Contar**, **Média**, **Máx**, **Mín** e outras.
5. Em **Adicionar subtotal a**, marque cada coluna a totalizar.
6. Deixe **Substituir subtotais atuais** marcada para trocar os subtotais que já existem.
7. **Resumo abaixo dos dados** começa como a estrutura de tópicos da planilha o define. Deixe-a marcada para colocar cada linha de total abaixo do seu grupo, ou desmarque-a para colocar cada linha de total acima do seu grupo, com o total geral no topo. Pressione **OK**.

Cada linha de total diz "Total Leste" (ou a entrada a que se refere) e contém uma fórmula SUBTOTAL, então ela deixa de fora as outras linhas de total. As linhas de cada grupo são agrupadas, para que você use os botões 1, 2 e 3 ao lado das linhas e mostre apenas os totais. Para remover os totais, abra **Subtotal** de novo e pressione **Remover Tudo**. Se os totais foram para o outro lado de uma planilha que já tinha seus próprios grupos, a barra de status informa que esses grupos agora têm suas linhas de resumo nesse lado, e Remover Tudo devolve o lado ao que era antes do primeiro subtotal, a menos que você tenha mudado isso manualmente depois. Quando os totais ficam acima, os botões da estrutura de tópicos ficam nas linhas de total, então um botão de mais ou de menos ao lado de "Total Leste" mostra ou oculta as linhas Leste abaixo dele. Se você desmarcar **Substituir subtotais atuais**, os novos totais devem ficar do mesmo lado dos que já existem. Caso contrário, nada é alterado, e uma mensagem informa isso. Acrescentar ou remover subtotais é uma etapa que você pode desfazer.

## Filtro Avançado

Os botões de Filtro escolhem linhas por uma coluna de cada vez. **Avançado** (no grupo **Classificar e Filtrar**) permite escrever as condições na planilha, para que sejam tão longas quanto for preciso.

1. Ao lado da lista, copie os cabeçalhos das colunas que você quer testar para uma linha de células vazias.
2. Abaixo de cada cabeçalho, escreva uma condição. As condições na **mesma linha** devem ser todas verdadeiras (isto é "e"). As condições em **linhas diferentes** são alternativas: uma linha da lista precisa atender a somente uma dessas linhas (isto é "ou").
3. Escolha **Avançado**. Confira se **Intervalo da lista** é a lista com sua linha de cabeçalho, e digite as células dos seus cabeçalhos e condições em **Intervalo de condições**.
4. Escolha **Ocultar as outras linhas na lista**, ou **Copiar as linhas para outro local** e digite a primeira célula do local, como H1. Marque **Ignorar linhas que repetem uma anterior** para copiar apenas uma de cada.
5. Pressione **OK**.

Como escrever uma condição:

- Uma palavra, como **An**, escolhe textos que começam com ela. Letras maiúsculas e minúsculas são consideradas iguais. **?** representa qualquer caractere e ***** qualquer sequência de caracteres.
- Um número escolhe células iguais a ele.
- **=Ana** escolhe textos exatamente iguais a Ana. **=** sozinho escolhe células vazias.
- **>**, **>=**, **<**, **<=** e **<>** (diferente) antes de um número ou de uma palavra fazem a comparação, como **>25**. **<>** sozinho escolhe células que não estão vazias.

Uma linha de condições sem nada escolhe todas as linhas. Para mostrar de novo as linhas ocultas, escolha **Limpar**. Uma cópia é feita na hora e não muda quando a lista muda. Filtro Avançado é uma etapa que você pode desfazer.

## Validação de Dados

Use isto para definir o que pode ser digitado em algumas células, para que uma entrada errada seja bloqueada ou questionada.

1. Selecione as células.
2. Escolha **Validação de Dados** no grupo **Ferramentas de Dados** da guia **Dados**.
3. Na guia **Configurações**, escolha o que **Permitir**: **Qualquer valor** (sem regra), **Número inteiro**, **Decimal**, **Lista**, **Data**, **Hora**, **Comprimento do texto** ou **Personalizado**.
4. Para um número, data, hora ou comprimento de texto, escolha **Dados** (**entre**, **não está entre**, **igual a** e assim por diante) e digite os limites. Um limite pode ser um valor ou uma fórmula que começa com =.
5. Para uma **Lista**, digite as opções separadas por vírgulas, como Vermelho,Verde,Azul, ou digite um intervalo de células depois de =, como =$A$1:$A$5. A célula então mostra uma seta com as opções. Desmarque **Lista suspensa na célula** para ocultar a seta.
6. Para **Personalizado**, digite uma fórmula que começa com =. A entrada é permitida quando a fórmula for verdadeira.
7. **Ignorar em branco** permite que uma célula vazia passe.

Na guia **Mensagem de Entrada**, marque a caixa e escreva um título e uma mensagem. Eles aparecem ao lado da célula sempre que ela for selecionada.

Na guia **Alerta de Erro**, escolha o que acontece com uma entrada não permitida. **Parar** a recusa. **Aviso** pergunta se deve mantê-la. **Informações** avisa a pessoa e mantém a entrada. Escreva um título e uma mensagem para o alerta.

**Circular Dados Inválidos** (ao lado de **Validação de Dados**) coloca um oval vermelho em cada célula da planilha cujo valor sua regra não permite, como um número que já estava ali antes da regra, ou o resultado de uma fórmula. Uma mensagem informa se todas as células estão corretas. Uma célula que você corrigiu perde o seu oval. **Limpar Círculos de Validação** remove todos os ovais.

**Limpar Tudo** remove as regras das células selecionadas. A nova regra substitui qualquer regra que as células selecionadas tivessem; as demais células mantêm as suas. Alterar regras é uma etapa que você pode desfazer, e as regras são salvas no arquivo.

## Consolidar

Use isto para combinar intervalos semelhantes, como a mesma tabela de várias planilhas, em uma única tabela.

1. Escolha **Consolidar** no grupo **Ferramentas de Dados** da guia **Dados**.
2. Escolha a **Função** com que combinar, como **Soma** ou **Média**.
3. Digite um intervalo na caixa e pressione **Adicionar**. Use **Planilha2!A1:C10** para um intervalo em outra planilha. Adicione cada intervalo que você quer combinar. Selecione um intervalo na lista e pressione **Excluir** para removê-lo.
4. Se os intervalos tiverem nomes na primeira linha, marque **A linha superior contém os nomes das colunas**. Se tiverem nomes na primeira coluna, marque **A coluna da esquerda contém os nomes das linhas**.
5. Em **Colocar o resultado em**, digite a primeira célula do resultado.
6. Marque **Criar vínculos com os dados de origem** se cada resultado deve ser uma fórmula que lê suas células de origem, para que mude quando elas mudarem. Sem essa opção, os resultados são números simples.
7. Pressione **OK**.

Sem nomes marcados, a célula na mesma posição de cada intervalo é combinada (a primeira célula de todos os intervalos, depois a segunda, e assim por diante). Com os nomes marcados, células com os mesmos nomes são combinadas, onde quer que estejam em cada intervalo, e um nome que só alguns intervalos têm ainda recebe uma linha ou coluna. Consolidar é uma etapa que você pode desfazer.

## Planilha de Previsão

A Planilha de Previsão cria uma nova planilha com uma tabela e um gráfico de valores futuros, calculados a partir de uma linha do tempo e de uma série de valores. Ela encontra a tendência e a sazonalidade repetida dos valores, e acrescenta uma faixa dentro da qual os valores futuros provavelmente ficarão.

1. Selecione a linha do tempo e os valores, duas colunas lado a lado (a linha do tempo primeiro). A linha do tempo são datas ou números com a mesma distância entre si, como todo mês ou a cada 5 unidades. Uma primeira linha de nomes é deixada de fora.
2. Escolha **Planilha de Previsão** no grupo **Previsão** da guia **Dados**.
3. Uma imagem mostra a previsão e muda conforme você altera as escolhas. Escolha **Gráfico de linhas** ou **Gráfico de colunas**.
4. Defina as escolhas:
   - **Intervalo da linha do tempo** e **Intervalo de valores** são as células a ler. Escreva outras células aqui se a seleção estiver errada.
   - **Início da previsão** fica vazio para começar depois do último ponto. Uma data ou número anterior ao último ponto começa ali e usa somente os pontos anteriores a ele, para que você possa comparar a previsão com o que aconteceu.
   - **Fim da previsão** é o último momento a prever. No máximo 10.000 pontos são previstos.
   - **Intervalo de confiança** indica quão seguro está a faixa, começando em 95%.
   - **Sazonalidade** é encontrada pela análise dos valores, ou você pode definir o tamanho da temporada em pontos, ou escolher nenhuma. Os valores devem conter duas temporadas completas.
   - **Preencher pontos ausentes usando** indica o que recebe um momento que não tem valor: uma linha reta entre os vizinhos, ou zero.
   - **Agregar duplicatas usando** indica como os valores no mesmo momento são combinados: Média, Contar, CONT.VALORES, Máx, Mediana, Mín ou Soma.
   - **Incluir estatísticas da previsão** acrescenta uma tabela com Alfa, Beta, Gama, MASE, SMAPE, MAE e RMSE, que indicam quão bem a previsão se ajusta aos valores.
5. Se algo estiver errado, a caixa de diálogo diz o que, em termos simples (passos que não são iguais, poucos pontos, texto nos valores), e **Criar** fica indisponível.
6. Pressione **Criar**.

A nova planilha é colocada antes da que você estava usando. Sua tabela tem as colunas Linha do tempo, Valores, Previsão, Limite inferior de confiança e Limite superior de confiança. A Previsão e os limites são fórmulas que usam `PREVISÃO.ETS` e `PREVISÃO.ETS.CONFINT` sobre as próprias colunas Linha do tempo e Valores da tabela, então são calculados de novo quando você altera um valor. O gráfico mostra os valores, a previsão e a faixa. Planilha de Previsão é uma etapa que você pode desfazer.

Os números podem diferir em alguns por cento dos de outros programas de planilha, que usam seu próprio modo de ajustar a suavização; e uma previsão que um arquivo já contém mantém os valores que o arquivo guardou, até que os dados que ela lê sejam alterados.

Um arquivo com mais de 1.000.000 de pontos não é usado.
