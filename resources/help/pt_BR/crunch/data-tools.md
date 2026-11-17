# Ferramentas de dados

<!-- keywords: formulário, formulário de dados, entrada de dados, registro, registros, novo registro, localizar próximo, localizar anterior, critérios, gerenciador de cenários, cenários, e se, hipótese, resumo de cenário, dados, guia dados, texto para colunas, dividir, separador, vírgula, tabulação, largura fixa, remover duplicatas, atingir meta, subtotal, totais, filtro avançado, condições, validação de dados, círculo dados inválidos, círculo vermelho, limpar círculos de validação, consolidar, combinar intervalos, permitir, lista, lista suspensa, mensagem de entrada, alerta de erro, linhas repetidas, exclusivo, arquivo de texto, csv, de texto, colunas, delimitador, aspas, editar links, atualizar valores, alterar origem, links, externo, origem, quebrar vínculo, verificar status, abrir origem -->

A guia **Dados** reúne as ferramentas que trabalham com uma lista de valores: obter texto de um arquivo, colocar uma lista em ordem, dividir texto em colunas e eliminar linhas repetidas.

## De Texto/CSV

**De Texto/CSV** abre um arquivo de texto (.csv, .tsv ou .txt). É o mesmo que **Abrir Arquivo de Texto** na guia Arquivo. O JeffCrunch pergunta como o arquivo está dividido em colunas e que tipo de texto ele contém, e então o mostra como uma nova pasta de trabalho.

## Editar Links

Se uma pasta de trabalho tiver fórmulas que leem outra pasta de trabalho (como ='[Preços.xlsx]Planilha1'!A1), **Editar Links**, no grupo **Obter Dados**, lista essas outras pastas de trabalho, chamadas origens. Cada linha mostra onde está a origem e o seu status. Até que a pasta de trabalho seja aberta novamente com as suas origens, ela mostra os números que o seu arquivo guardou a partir delas.

- **Verificar Status** procura cada origem neste computador e informa **Encontrada**, ou informa que ela não foi encontrada. Uma origem na web não é buscada, portanto o status indica que ela não foi verificada.
- **Atualizar Valores** lê a origem escolhida deste computador e dá a cada célula que é apenas uma referência a uma de suas células o valor que essa célula tem agora. Uma célula que faz mais com o valor, como `=[1]Preços!A1*2`, mantém o número que o seu arquivo guardou, e a mensagem sob a lista informa quantas células foram atualizadas e quantas não foram. Atualizar Valores é uma única etapa, portanto **Desfazer** restaura os valores antigos. Os novos valores também são gravados na parte do próprio vínculo no arquivo ao salvar, de modo que a pasta de trabalho os mostra na próxima vez que for aberta, mesmo quando a origem não estiver disponível.
- **Alterar Origem** permite escolher outro arquivo para o vínculo selecionado, por exemplo quando a pasta de trabalho foi movida. As fórmulas permanecem como estão. O novo local é gravado no arquivo ao salvar, e **Desfazer** desfaz a alteração.
- **Abrir Origem** abre a origem escolhida em uma nova janela, quando ela puder ser encontrada.
- **Quebrar Vínculo** transforma cada célula que lê a origem escolhida no valor que ela mostra agora, e um nome que representava um local nela em **#REF!**. O JeffCrunch pede confirmação antes. Quebrar Vínculo é uma única etapa, portanto **Desfazer** restaura as fórmulas, os nomes e os vínculos, até que você salve a pasta de trabalho. Quando não restar nenhum vínculo, o arquivo é salvo sem as partes que os guardavam.

## Formulário

**Formulário** (no grupo **Ferramentas de Dados** e na caixa de pesquisa) mostra uma linha de uma lista por vez, como um pequeno formulário. Uma lista é uma tabela, ou um bloco de células com uma linha de nomes no topo e uma linha de dados abaixo de cada nome. Clique primeiro em qualquer célula da lista.

- Cada caixa corresponde a uma coluna, identificada pelo nome na linha superior. Digite em uma caixa para alterar o valor. Uma caixa em cinza contém uma fórmula: o formulário mostra o resultado, e você altera a fórmula na planilha.
- **Localizar Próximo** e **Localizar Anterior** vão para a linha seguinte e para a linha anterior. Ir para outra linha, ou pressionar **Enter**, grava o que você digitou nesta linha.
- **Novo** mostra um formulário vazio. Preencha-o e pressione **Novo** de novo, **Localizar Próximo** ou **Fechar**. A linha é adicionada abaixo da última linha da lista. Uma tabela cresce uma linha. Se uma célula abaixo da lista não estiver vazia, o formulário avisa e não adiciona nada.
- **Excluir** remove a linha que você está vendo, depois de pedir confirmação. As linhas abaixo dela sobem. Uma tabela sempre mantém pelo menos uma linha de dados. Uma linha oculta, ou que um filtro oculta, não é mostrada no formulário, não é contada e nunca é excluída por ele. Se a lista for longa demais para ser percorrida por inteiro, o formulário informa que mostra apenas as primeiras linhas.
- **Restaurar** desfaz o que você digitou nesta linha, antes de sair dela.
- **Critérios** permite procurar linhas. Digite uma condição em uma ou mais caixas e pressione **Localizar Próximo** ou **Localizar Anterior**. Uma condição é um valor, como **Ana** (o texto corresponde ao início da palavra, e **?** e ***** representam letras), ou um valor precedido de um sinal: **>100**, **<=5**, **<>0**, **=Oslo**. Uma linha deve atender a todas as condições digitadas. Pressione **Formulário** para voltar, ou **Limpar** para recomeçar. Enquanto houver critérios definidos, **Localizar Próximo** e **Localizar Anterior** pulam as linhas que não os atendem.
- **Fechar** grava a linha e fecha o formulário.

Cada linha que você altera, adiciona ou exclui é uma etapa, portanto **Desfazer** na planilha a desfaz de uma só vez.

## Classificar e filtrar

O grupo **Classificar e Filtrar** tem os mesmos comandos do menu **Classificar e Filtrar** da guia Página Inicial. Veja [Classificar e filtrar](sort-filter).

## Texto para Colunas

Use isto quando uma coluna contém vários trechos de texto que você quer em colunas separadas, como "Silva, Ana, 31".

1. Selecione as células de **uma** coluna.
2. Escolha **Texto para Colunas** na guia **Dados**.
3. Escolha como dividir o texto:
   - **Dividir em um separador**: marque os caracteres que ficam entre os trechos: **Tabulação**, **Ponto e vírgula**, **Vírgula**, **Espaço** ou **Outro**, em que você digita o seu próprio caractere. **Tratar separadores consecutivos como um só** impede que duas vírgulas seguidas criem um trecho vazio. **Texto entre aspas** indica o caractere que envolve um trecho que não deve ser dividido, como "Lee, Bo". Dentro das aspas, dois caracteres seguidos representam um só.
   - **Dividir em posições fixas**: digite as posições em que uma nova coluna começa, contando os caracteres a partir de 1. Por exemplo, 5, 12 inicia uma nova coluna no quinto e no décimo segundo caractere.
4. Escolha o que cada trecho é: **Geral** lê números e datas como se fossem digitados, e **Texto** mantém cada trecho como está, de modo que "007" continua "007".
5. Em **Colocar os trechos a partir de**, digite a primeira célula para os trechos. Ela começa como a primeira célula selecionada.
6. A caixa abaixo das opções mostra as primeiras linhas divididas da forma escolhida. Pressione **OK**.

Os trechos vão para as células à direita da primeira célula e substituem o que havia nelas. Texto para Colunas é uma única etapa que você pode desfazer.

## Remover Duplicatas

Use isto para manter apenas a primeira de cada grupo de linhas que se repetem.

1. Selecione as células da lista, ou uma célula dentro dela. O JeffCrunch usa todo o bloco de dados ao redor de uma única célula.
2. Escolha **Remover Duplicatas**.
3. Marque **Meus dados têm cabeçalhos** se a primeira linha contiver nomes. Essa linha permanece e não é comparada.
4. Marque as colunas que devem coincidir para que duas linhas sejam consideradas repetidas. Duas linhas se repetem somente quando todas as colunas marcadas são iguais. Letras maiúsculas e minúsculas são tratadas como iguais, e uma fórmula conta pelo valor que produz.
5. Pressione **OK**. A caixa de diálogo informa quantas linhas foram removidas e quantas restaram.

As linhas abaixo de uma linha removida sobem dentro das células selecionadas, com as suas formatações e fórmulas. As células abaixo da seleção não se movem. As linhas vazias deixadas na parte inferior da seleção são limpas. Remover Duplicatas é uma única etapa que você pode desfazer.

## Atingir Meta

Use isto quando você sabe o resultado que quer, mas não o número que o produz. Por exemplo, uma célula de pagamento contém uma fórmula que usa uma taxa de juros, e você quer que o pagamento seja 500. Atingir Meta testa números na célula da taxa até que o pagamento seja 500.

1. Escolha **Atingir Meta** na guia **Dados**.
2. Em **Célula com a fórmula**, digite a célula que contém a fórmula, como B3. Ela começa como a célula selecionada.
3. Em **Valor desejado**, digite o resultado que você quer que a fórmula produza.
4. Em **Célula a alterar**, digite a célula que Atingir Meta pode alterar, como B1. Ela deve conter um número, não uma fórmula.
5. Pressione **OK**.

Se Atingir Meta encontrar um número, ele permanece na célula a alterar, e a barra de status informa qual é. Se não conseguir, nada muda e você é avisado. Tente outro número inicial na célula a alterar e confira se a fórmula realmente usa essa célula. Atingir Meta é uma única etapa que você pode desfazer.

## Gerenciador de Cenários

Um cenário é um conjunto nomeado de valores para algumas células. Use cenários para testar "e se" em uma planilha. Por exemplo, mantenha um cenário "Vendas baixas" e outro "Vendas altas" para as mesmas células de entrada, e mostre aquele que você quer examinar.

1. Escolha **Gerenciador de Cenários** na guia **Dados**.
2. Pressione **Adicionar**. Digite um nome, como Vendas altas. Em **Células que serão alteradas**, digite as células que o cenário define, como B1:B3 ou D5. Um cenário pode alterar até 32 células. As células que você selecionou são preenchidas para você. Cada célula começa com o valor que contém agora. Digite na tabela o valor que este cenário deve usar e pressione **OK**.
3. Escolha um cenário na lista e pressione **Mostrar** para colocar os seus valores nas células. Clicar duas vezes em um nome faz o mesmo. Pressione **Editar** para alterar o seu nome, o seu comentário ou os seus valores. Pressione **Excluir** para removê-lo.

Mostrar, Adicionar, Editar e Excluir são, cada um, uma etapa que você pode desfazer.

**Resumo** coloca todos os cenários lado a lado em uma nova planilha chamada Resumo do Cenário. Ele pede as células de resultado, que são as células que você quer comparar, como o total em B10. Para cada cenário, o resumo mostra os valores das células alteradas e o resultado das células de resultado, calculados com os valores daquele cenário. Uma coluna chamada Valores Atuais mostra como as células estavam quando você criou o resumo. As suas próprias células são deixadas como estavam.

Os cenários são salvos na pasta de trabalho. Uma pasta de trabalho feita por outro programa mantém os seus cenários, e você os vê aqui. Uma cópia salva como arquivo .xls mais antigo ou como .ods não os guarda, e o salvamento informa isso.

## Subtotal

Use isto para colocar uma linha de totais abaixo de cada grupo de linhas que têm a mesma entrada em uma coluna, e um total geral na parte inferior. A lista precisa de uma linha de cabeçalho e deve estar classificada pela coluna que você agrupa, para que entradas iguais fiquem próximas.

1. Selecione uma célula na lista.
2. Escolha **Subtotal**, no grupo **Estrutura de Tópicos** da guia **Dados**.
3. Em **A cada alteração em**, escolha a coluna que agrupa as linhas, como Região.
4. Em **Usar função**, escolha o que calcular: **Soma**, **Contar**, **Média**, **Máx**, **Mín** e outras.
5. Em **Adicionar subtotal a**, marque cada coluna a totalizar.
6. Deixe **Substituir subtotais atuais** marcado para trocar os subtotais que já existem. Pressione **OK**.

Cada linha de total diz "Total Leste" (ou a entrada a que se refere) e contém uma fórmula SUBTOTAL, de modo que ignora as outras linhas de total. As linhas de cada grupo são agrupadas, para que você possa usar os botões 1, 2 e 3 ao lado das linhas e mostrar apenas os totais. Para remover os totais, abra **Subtotal** novamente e pressione **Remover Tudo**. Adicionar ou remover subtotais é uma única etapa que você pode desfazer.

## Filtro Avançado

Os botões de filtro escolhem linhas por uma coluna de cada vez. **Avançado** (no grupo **Classificar e Filtrar**) permite escrever as condições na planilha, para que sejam tão longas quanto precisar.

1. Ao lado da lista, copie os cabeçalhos das colunas que quer testar para uma linha de células vazias.
2. Abaixo de cada cabeçalho, escreva uma condição. As condições na **mesma linha** precisam ser todas verdadeiras (isto é "e"). As condições em **linhas diferentes** são alternativas: uma linha da lista precisa atender a apenas uma dessas linhas (isto é "ou").
3. Escolha **Avançado**. Confira se **Intervalo da lista** é a lista com a sua linha de cabeçalho, e digite as células dos seus cabeçalhos e condições em **Intervalo de condições**.
4. Escolha **Ocultar as outras linhas na lista**, ou **Copiar as linhas para outro local** e digite a primeira célula do local, como H1. Marque **Omitir linhas que repetem uma anterior** para copiar apenas uma de cada.
5. Pressione **OK**.

Como escrever uma condição:

- Uma palavra, como **An**, escolhe textos que começam com ela. Letras maiúsculas e minúsculas são tratadas como iguais. **?** representa qualquer caractere, e ***** qualquer sequência de caracteres.
- Um número escolhe as células iguais a ele.
- **=Ana** escolhe textos exatamente iguais a Ana. **=** sozinho escolhe células vazias.
- **>**, **>=**, **<**, **<=** e **<>** (diferente) antes de um número ou de uma palavra fazem a comparação, como **>25**. **<>** sozinho escolhe células que não estão vazias.

Uma linha de condições sem nada nela escolhe todas as linhas. Para mostrar novamente as linhas ocultas, escolha **Limpar**. Uma cópia é feita na hora e não muda quando a lista muda. Filtro Avançado é uma única etapa que você pode desfazer.

## Validação de Dados

Use isto para definir o que pode ser digitado em algumas células, de modo que uma entrada incorreta seja bloqueada ou questionada.

1. Selecione as células.
2. Escolha **Validação de Dados**, no grupo **Ferramentas de Dados** da guia **Dados**.
3. Na guia **Configurações**, escolha o que **Permitir**: **Qualquer valor** (sem regra), **Número inteiro**, **Decimal**, **Lista**, **Data**, **Hora**, **Comprimento do texto** ou **Personalizado**.
4. Para um número, data, hora ou comprimento de texto, escolha **Dados** (**entre**, **não está entre**, **igual a** e assim por diante) e digite os limites. Um limite pode ser um valor ou uma fórmula que começa com =.
5. Para uma **Lista**, digite as opções separadas por vírgulas, como Vermelho,Verde,Azul, ou digite um intervalo de células depois de =, como =$A$1:$A$5. A célula então mostra uma seta com as opções. Desative **Lista suspensa na célula** para ocultar a seta.
6. Para **Personalizado**, digite uma fórmula que começa com =. A entrada é permitida quando a fórmula é verdadeira.
7. **Ignorar em branco** permite que uma célula vazia passe.

Na guia **Mensagem de Entrada**, marque a caixa e escreva um título e uma mensagem. Eles são mostrados ao lado da célula sempre que ela estiver selecionada.

Na guia **Alerta de Erro**, escolha o que acontece com uma entrada não permitida. **Parar** a recusa. **Aviso** pergunta se você quer mantê-la. **Informações** avisa a pessoa e mantém a entrada. Escreva um título e uma mensagem para o alerta.

**Circular Dados Inválidos** (ao lado de **Validação de Dados**) coloca um círculo vermelho em cada célula da planilha cujo valor a sua regra não permite, como um número que já estava ali antes da regra, ou o resultado de uma fórmula. Uma mensagem informa se todas as células estão corretas. Uma célula que você corrigir perde o seu círculo. **Limpar Círculos de Validação** remove todos os círculos.

**Limpar Tudo** remove as regras das células selecionadas. A nova regra substitui qualquer regra que as células selecionadas tivessem; as outras células mantêm as delas. Alterar regras é uma única etapa que você pode desfazer, e as regras são salvas no arquivo.

## Consolidar

Use isto para combinar intervalos semelhantes, como a mesma tabela de várias planilhas, em uma única tabela.

1. Escolha **Consolidar**, no grupo **Ferramentas de Dados** da guia **Dados**.
2. Escolha a **Função** com que combinar, como **Soma** ou **Média**.
3. Digite um intervalo na caixa e pressione **Adicionar**. Use **Planilha2!A1:C10** para um intervalo em outra planilha. Adicione cada intervalo que quer combinar. Selecione um intervalo na lista e pressione **Excluir** para removê-lo.
4. Se os intervalos tiverem nomes na primeira linha, marque **A linha superior contém os nomes das colunas**. Se tiverem nomes na primeira coluna, marque **A coluna da esquerda contém os nomes das linhas**.
5. Em **Colocar o resultado em**, digite a primeira célula do resultado.
6. Marque **Criar vínculos com os dados de origem** se cada resultado deve ser uma fórmula que lê as suas células de origem, para que mude quando elas mudarem. Sem essa opção, os resultados são números simples.
7. Pressione **OK**.

Sem nomes marcados, combina-se a célula na mesma posição de cada intervalo (a primeira célula de cada intervalo, depois a segunda, e assim por diante). Com nomes marcados, combinam-se as células com os mesmos nomes, onde quer que estejam em cada intervalo, e um nome que apenas alguns intervalos têm ainda recebe uma linha ou coluna. Consolidar é uma única etapa que você pode desfazer.
