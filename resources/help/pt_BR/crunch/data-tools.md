# Ferramentas de Dados

<!-- keywords: gerenciador de cenários, cenários, e se, o que acontece se, resumo de cenário, dados, guia dados, texto para colunas, dividir, separador, vírgula, tabulação, largura fixa, remover duplicatas, atingir meta, subtotal, totais, filtro avançado, condições, validação de dados, circular dados inválidos, círculo vermelho, limpar círculos de validação, consolidar, combinar intervalos, permitir, lista, suspensa, mensagem de entrada, alerta de erro, linhas repetidas, exclusivo, arquivo de texto, csv, de texto, colunas, delimitador, aspas, editar links, atualizar valores, alterar origem, links, externo, origem, quebrar vínculo, verificar status, abrir origem -->

A guia **Dados** tem as ferramentas que trabalham com uma lista de valores: obter texto de um arquivo, colocar uma lista em ordem, dividir texto em colunas e excluir linhas repetidas.

## De Texto/CSV

**De Texto/CSV** abre um arquivo de texto (.csv, .tsv ou .txt). É o mesmo que **Abrir Arquivo de Texto** na guia Arquivo. O JeffCrunch pergunta como o arquivo está dividido em colunas e que tipo de texto ele contém, e depois o mostra em uma nova pasta de trabalho.

## Editar Links

Se uma pasta de trabalho tiver fórmulas que leem outra pasta de trabalho (como ='[Preços.xlsx]Planilha1'!A1), **Editar Links**, no grupo **Obter Dados**, lista essas outras pastas de trabalho, chamadas de origens. Cada linha mostra onde está a origem e o seu status. Até que a pasta de trabalho seja aberta novamente com as suas origens, ela mostra os números que o arquivo armazenou a partir delas.

- **Verificar Status** procura cada origem neste computador e informa **Encontrada**, ou informa que ela não foi encontrada. Uma origem na web não é baixada, e o status dela indica que não foi verificada.
- **Atualizar Valores** lê a origem escolhida deste computador e dá a cada célula que é apenas uma referência a uma de suas células o valor que essa célula tem agora. Uma célula que faz algo mais com o valor, como `=[1]Preços!A1*2`, mantém o número que o arquivo armazenou, e a mensagem sob a lista informa quantas células foram atualizadas e quantas não foram. Atualizar Valores é uma única etapa, então **Desfazer** restaura os valores antigos.
- **Alterar Origem** permite escolher outro arquivo para o vínculo selecionado, por exemplo quando a pasta de trabalho foi movida. As fórmulas permanecem como estão. O novo local é gravado no arquivo ao salvar, e **Desfazer** desfaz a alteração.
- **Abrir Origem** abre a origem escolhida em uma nova janela, quando ela pode ser encontrada.
- **Quebrar Vínculo** transforma cada célula que lê a origem escolhida no valor que ela mostra agora, e um nome que representava um local nela em **#REF!**. Isso não pode ser desfeito, então o JeffCrunch pede confirmação antes. Quando não sobra nenhum link, o arquivo é salvo sem as partes que os continham.

## Classificar e filtrar

O grupo **Classificar e Filtrar** tem os mesmos comandos do menu **Classificar e Filtrar** na guia Página Inicial. Veja [Classificar e filtrar](sort-filter).

## Texto para Colunas

Use isto quando uma coluna tiver vários pedaços de texto que você quer em colunas separadas, como "Silva, Ana, 31".

1. Selecione as células de **uma** coluna.
2. Escolha **Texto para Colunas** na guia **Dados**.
3. Escolha como dividir o texto:
   - **Dividir em um separador**: marque os caracteres que ficam entre as partes: **Tabulação**, **Ponto e vírgula**, **Vírgula**, **Espaço** ou **Outro**, em que você digita o seu próprio caractere. **Tratar separadores consecutivos como um só** impede que duas vírgulas seguidas criem uma parte vazia. **Texto entre aspas** indica o caractere que envolve uma parte que não deve ser dividida, como "Lee, Bo". Dentro das aspas, dois caracteres seguidos valem por um.
   - **Dividir em posições fixas**: digite as posições em que uma nova coluna começa, contando os caracteres a partir de 1. Por exemplo, 5, 12 inicia uma nova coluna no quinto e no décimo segundo caractere.
4. Escolha o tipo de cada parte: **Geral** lê números e datas como se você os tivesse digitado, e **Texto** mantém cada parte como está, para que "007" continue "007".
5. Em **Colocar as partes a partir de**, digite a primeira célula para as partes. Ela começa como a primeira célula que você selecionou.
6. A caixa sob as opções mostra as primeiras linhas divididas do jeito que você escolheu. Pressione **OK**.

As partes vão para as células à direita da primeira célula, e substituem o que havia nelas. Texto para Colunas é uma única etapa que você pode desfazer.

## Remover Duplicatas

Use isto para manter apenas a primeira de cada grupo de linhas que se repetem.

1. Selecione as células da lista, ou uma célula dela. O JeffCrunch usa todo o bloco de dados ao redor de uma única célula.
2. Escolha **Remover Duplicatas**.
3. Marque **Meus dados têm linha de cabeçalho** se a primeira linha tiver nomes. Essa linha é mantida e não é comparada.
4. Marque as colunas que precisam ser iguais para que duas linhas sejam consideradas repetidas. Duas linhas se repetem apenas quando todas as colunas marcadas são iguais. Letras maiúsculas e minúsculas são tratadas como iguais, e uma fórmula conta pelo valor que ela produz.
5. Pressione **OK**. A caixa informa quantas linhas foram removidas e quantas restam.

As linhas abaixo de uma linha removida sobem dentro das células selecionadas, com as suas formatações e fórmulas. As células abaixo da seleção não se movem. As linhas vazias que sobram no fim da seleção são limpas. Remover Duplicatas é uma única etapa que você pode desfazer.

## Atingir Meta

Use isto quando você sabe o resultado que quer, mas não o número que o produz. Por exemplo, uma célula de pagamento tem uma fórmula que usa uma taxa de juros, e você quer que o pagamento seja 500. Atingir Meta testa números na célula da taxa até que o pagamento seja 500.

1. Escolha **Atingir Meta** na guia **Dados**.
2. Em **Célula com a fórmula**, digite a célula que contém a fórmula, como B3. Ela começa como a célula que você selecionou.
3. Em **Valor desejado**, digite o resultado que a fórmula deve dar.
4. Em **Célula a alterar**, digite a célula que Atingir Meta pode alterar, como B1. Ela deve conter um número, não uma fórmula.
5. Pressione **OK**.

Se Atingir Meta encontrar um número, ele fica na célula a alterar, e a barra de status informa qual é. Se não conseguir, nada muda e você é avisado. Tente um número inicial diferente na célula a alterar e confirme se a fórmula realmente usa essa célula. Atingir Meta é uma única etapa que você pode desfazer.

## Gerenciador de Cenários

Um cenário é um conjunto nomeado de valores para algumas células. Use cenários para testar o "e se" em uma planilha. Por exemplo, mantenha um cenário "Vendas baixas" e um cenário "Vendas altas" para as mesmas células de entrada, e mostre aquele que deseja analisar.

1. Escolha **Gerenciador de Cenários** na guia **Dados**.
2. Pressione **Adicionar**. Digite um nome, como Vendas altas. Em **Células alteradas**, digite as células que o cenário define, como B1:B3 ou D5. Um cenário pode alterar até 32 células. As células que você selecionou são preenchidas para você. Cada célula começa com o valor que tem agora. Digite na tabela o valor que este cenário deve usar e pressione **OK**.
3. Escolha um cenário na lista e pressione **Mostrar** para colocar os seus valores nas células. Clicar duas vezes em um nome faz o mesmo. Pressione **Editar** para alterar o nome, o comentário ou os valores. Pressione **Excluir** para removê-lo.

Mostrar, Adicionar, Editar e Excluir são, cada um, uma etapa que você pode desfazer.

**Resumo** coloca todos os cenários lado a lado em uma nova planilha chamada Resumo de Cenário. Ele pede as células de resultado, que são as células que você quer comparar, como o total em B10. Para cada cenário, o resumo mostra os valores das células alteradas e o que as células de resultado dão, calculado com os valores daquele cenário. Uma coluna chamada Valores Atuais mostra como estavam as células quando você criou o resumo. As suas próprias células são mantidas como estavam.

Os cenários são salvos na pasta de trabalho. Uma pasta de trabalho criada por outro programa mantém os seus cenários, e você os vê aqui. Uma cópia salva como arquivo .xls mais antigo ou como .ods não os contém, e o salvamento avisa você.

## Subtotal

Use isto para colocar uma linha de totais sob cada grupo de linhas que têm a mesma entrada em uma coluna, e um total geral no final. A lista precisa de uma linha de cabeçalho e deve estar classificada pela coluna que você agrupa, para que entradas iguais fiquem próximas.

1. Selecione uma célula da lista.
2. Escolha **Subtotal** no grupo **Estrutura de Tópicos** da guia **Dados**.
3. Em **A cada alteração em**, escolha a coluna que agrupa as linhas, como Região.
4. Em **Usar função**, escolha o que calcular: **Soma**, **Contagem**, **Média**, **Máximo**, **Mínimo** e outras.
5. Em **Adicionar subtotal a**, marque cada coluna a totalizar.
6. Deixe **Substituir subtotais atuais** marcado para trocar os subtotais que já existem. Pressione **OK**.

Cada linha de total diz "Total Leste" (ou a entrada a que se refere) e contém uma fórmula SUBTOTAL, para que ignore as outras linhas de total. As linhas de cada grupo ficam agrupadas, então você pode usar os botões 1, 2 e 3 ao lado das linhas para mostrar somente os totais. Para remover os totais, abra **Subtotal** novamente e pressione **Remover Tudo**. Adicionar ou remover subtotais é uma única etapa que você pode desfazer.

## Filtro Avançado

Os botões de Filtro escolhem linhas por uma coluna de cada vez. **Avançado** (no grupo **Classificar e Filtrar**) permite escrever as condições na planilha, para que elas tenham o tamanho que você precisar.

1. Ao lado da lista, copie os cabeçalhos das colunas que você quer testar para uma linha de células vazias.
2. Abaixo de cada cabeçalho, escreva uma condição. As condições na **mesma linha** precisam ser todas verdadeiras (isto é "e"). As condições em **linhas diferentes** são alternativas: uma linha da lista precisa atender a apenas uma dessas linhas (isto é "ou").
3. Escolha **Avançado**. Confirme que **Intervalo da lista** é a lista com a sua linha de cabeçalho, e digite as células dos seus cabeçalhos e condições em **Intervalo das condições**.
4. Escolha **Ocultar as outras linhas na lista**, ou **Copiar as linhas para outro local** e digite a primeira célula do local, como H1. Marque **Omitir linhas que repetem uma anterior** para copiar apenas uma de cada.
5. Pressione **OK**.

Como escrever uma condição:

- Uma palavra, como **Ana**, escolhe textos que começam com ela. Letras maiúsculas e minúsculas são tratadas como iguais. **?** representa qualquer caractere e ***** qualquer sequência de caracteres.
- Um número escolhe células iguais a ele.
- **=Ana** escolhe textos exatamente iguais a Ana. **=** sozinho escolhe células vazias.
- **>**, **>=**, **<**, **<=** e **<>** (diferente) antes de um número ou de uma palavra fazem a comparação, como **>25**. **<>** sozinho escolhe células que não estão vazias.

Uma linha de condições sem nada dentro escolhe todas as linhas. Para mostrar novamente as linhas ocultas, escolha **Limpar**. Uma cópia é feita na hora e não muda quando a lista muda. Filtro Avançado é uma única etapa que você pode desfazer.

## Validação de Dados

Use isto para definir o que pode ser digitado em algumas células, de modo que uma entrada errada seja bloqueada ou questionada.

1. Selecione as células.
2. Escolha **Validação de Dados** no grupo **Ferramentas de Dados** da guia **Dados**.
3. Na guia **Configurações**, escolha o que **Permitir**: **Qualquer valor** (sem regra), **Número inteiro**, **Decimal**, **Lista**, **Data**, **Hora**, **Comprimento do texto** ou **Personalizado**.
4. Para número, data, hora ou comprimento do texto, escolha **Dados** (**entre**, **não entre**, **igual a** e assim por diante) e digite os limites. Um limite pode ser um valor ou uma fórmula que começa com =.
5. Para uma **Lista**, digite as opções separadas por vírgulas, como Vermelho,Verde,Azul, ou digite um intervalo de células após =, como =$A$1:$A$5. A célula então mostra uma seta com as opções. Desmarque **Lista suspensa na célula** para ocultar a seta.
6. Para **Personalizado**, digite uma fórmula que começa com =. A entrada é aceita quando a fórmula é verdadeira.
7. **Ignorar vazio** permite que uma célula vazia passe.

Na guia **Mensagem de Entrada**, marque a caixa e escreva um título e uma mensagem. Eles aparecem ao lado da célula sempre que ela for selecionada.

Na guia **Alerta de Erro**, escolha o que acontece com uma entrada não permitida. **Parar** a recusa. **Aviso** pergunta se deve mantê-la. **Informações** avisa a pessoa e mantém a entrada. Escreva um título e uma mensagem para o alerta.

**Circular Dados Inválidos** (ao lado de **Validação de Dados**) coloca um círculo vermelho em cada célula da planilha cujo valor sua regra não permite, como um número que já estava lá antes da regra, ou o resultado de uma fórmula. Uma mensagem avisa se todas as células estiverem corretas. Uma célula que você corrigir perde o círculo. **Limpar Círculos de Validação** remove todos os círculos.

**Limpar Tudo** retira as regras das células selecionadas. A nova regra substitui qualquer regra que as células selecionadas tivessem; as outras células mantêm as suas. Alterar regras é uma única etapa que você pode desfazer, e as regras são salvas no arquivo.

## Consolidar

Use isto para combinar intervalos semelhantes, como a mesma tabela de várias planilhas, em uma única tabela.

1. Escolha **Consolidar** no grupo **Ferramentas de Dados** da guia **Dados**.
2. Escolha a **Função** a usar na combinação, como **Soma** ou **Média**.
3. Digite um intervalo na caixa e pressione **Adicionar**. Use **Planilha2!A1:C10** para um intervalo em outra planilha. Adicione cada intervalo que quer combinar. Selecione um intervalo na lista e pressione **Excluir** para removê-lo.
4. Se os intervalos tiverem nomes na primeira linha, marque **Linha superior contém os nomes das colunas**. Se tiverem nomes na primeira coluna, marque **Coluna da esquerda contém os nomes das linhas**.
5. Em **Colocar o resultado em**, digite a primeira célula do resultado.
6. Marque **Criar vínculos com os dados de origem** se cada resultado deve ser uma fórmula que lê as suas células de origem, de modo que mude quando elas mudarem. Sem essa opção, os resultados são números simples.
7. Pressione **OK**.

Sem nomes marcados, a célula na mesma posição de cada intervalo é combinada (a primeira célula de todos os intervalos, depois a segunda, e assim por diante). Com os nomes marcados, células com os mesmos nomes são combinadas, estejam onde estiverem em cada intervalo, e um nome que só alguns intervalos têm ainda recebe uma linha ou coluna. Consolidar é uma única etapa que você pode desfazer.
