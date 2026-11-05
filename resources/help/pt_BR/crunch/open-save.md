# Abrir e salvar pastas de trabalho
<!-- keywords: copiar, formato antigo, opendocument, compatibilidade, novo, em branco, pasta1, sem título, abrir, salvar, salvar como, arquivo, pasta de trabalho, xlsx, xlsm, xls, ods, csv, tsv, arquivo de texto, delimitado, importar, exportar, recente, fixado, arrastar, soltar, carregando, grande, macros, não salvo, alterações, fechar, cópia, cadeia de cálculo, modelo, xltx, xltm, não é possível abrir, danificado, criptografado, senha, salvar uma cópia, somente leitura, abrir como cópia, nova janela, cópia de backup, meus modelos, salvar como modelo, ots, reparar, abrir e reparar, modelos prontos, orçamento, fatura, folha de horas, empréstimo, calendário, diário de notas -->

Quando o JeffCrunch inicia sem arquivo para abrir, a janela mostra uma pasta de trabalho nova e vazia, com uma planilha chamada Pasta1. Cada nova pasta de trabalho recebe o próximo número (Pasta2, Pasta3, e assim por diante), e um número não é usado novamente depois que a janela é fechada. Para iniciar outra, pressione **{key:New}**, ou clique em **Arquivo** e depois em **Novo**, e escolha **Pasta de trabalho em branco**; ela abre em uma janela própria. Uma pasta de trabalho nova que você não alterou dá lugar quando você abre um arquivo na sua janela; uma que você alterou permanece, e o arquivo é aberto em outra janela. **Salvar** em uma pasta de trabalho nova pede um nome, assim como **Salvar Como**, e a grava como arquivo .xlsx. Fechar uma pasta de trabalho nova que você não alterou não pede nada.

Para abrir uma pasta de trabalho, pressione **{key:Open}**, ou clique em **Arquivo** e depois em **Abrir**, e escolha **Procurar**. Você também pode arrastar uma pasta de trabalho do gerenciador de arquivos para a janela. Uma pasta de trabalho que já está aberta em uma janela não é aberta uma segunda vez: essa janela é trazida para a frente.

O JeffCrunch abre pastas de trabalho **.xlsx** e pastas de trabalho **.xlsm** (as que contêm macros), e os modelos **.xltx** e **.xltm** (um modelo que contém macros). Ele não executa macros e as mantém intactas: o projeto de macros, sua assinatura e as partes que os referenciam são gravados exatamente como estavam. Um modelo é salvo como modelo, e uma pasta de trabalho com macros como pasta de trabalho com macros, porque o tipo faz parte do arquivo.

## Outros tipos de arquivo

O JeffCrunch também abre pastas de trabalho **.xls** mais antigas, planilhas OpenDocument (**.ods** e a forma simples **.fods**) e arquivos de texto com valores (**.csv**, **.tsv** e **.txt**). Uma pasta de trabalho aberta a partir de um deles é salva como novo arquivo **.xlsx** (ou como cópia **.xls** ou **.ods**: veja abaixo). Quando você pressiona **{key:Save}**, o JeffCrunch avisa isso e pede um nome, e o arquivo que você abriu nunca é alterado.

Uma planilha OpenDocument é lida com as suas planilhas, valores, fórmulas (com os resultados que o arquivo guardou), formatos de número, fontes, preenchimentos, bordas, células mescladas, larguras de coluna e alturas de linha, linhas e colunas ocultas, painéis congelados e nomes. Gráficos, imagens e outros desenhos, comentários, tabelas dinâmicas, macros, formatações condicionais e validação de dados ainda não são lidos; uma mensagem no topo informa quais desses o arquivo tem, e uma cópia salva a partir daqui não os terá. Uma planilha protegida por senha pede a senha ao ser aberta: veja [Abrir pastas de trabalho protegidas por senha](open-password).

Um arquivo de texto é uma planilha. O JeffCrunch descobre como os valores são separados (por vírgulas, ponto e vírgula, tabulações ou barras verticais) e como as letras são codificadas (UTF-8 ou UTF-16, ou então a página de código do seu idioma), e lê cada valor como você o digitaria no seu idioma: um número como 1.234,5 em alemão, uma porcentagem, um valor monetário, uma data com ano (como 15/03/2024, ou 15.03.2024 em alemão), uma hora, VERDADEIRO ou FALSO (no seu idioma, como WAHR e FALSCH em alemão, e em inglês). Um valor que começa com sinal de igual é uma fórmula. Números que começam com zero (como 007) e números com mais de 15 dígitos continuam sendo texto, para que um CEP ou um número de conta não seja alterado; o mesmo vale para dia e mês sem ano, em qualquer ordem e em qualquer idioma (como 3/4, 12,5, 1-2, 1 de março ou 25 dez), porque o ano teria de ser inventado, e para textos como o nome de gene MARCH1. Uma planilha tem 1.048.576 linhas e 16.384 colunas; se o arquivo tiver mais, o JeffCrunch abre o que cabe e informa isso.

Ao abrir um arquivo de texto, uma janela mostra primeiro as suas primeiras linhas e pergunta como ele deve ser lido: o que separa os campos (qualquer um entre vírgula, ponto e vírgula, tabulação, espaço, barra vertical ou um caractere que você digitar, com vários seguidos contados como um, se você quiser), o que delimita um campo que contém um separador, a codificação do texto (Unicode, as páginas de código do Windows, as páginas ISO 8859, Mac Roman e as do Leste Asiático) e a linha em que começar. Acima de cada coluna você escolhe como ela é lida: **Como digitado**, **Texto** (para manter um código de peça como 007 ou um número de conta longo como está), **Data** na ordem que você indicar, ou **Ignorar**. A prévia acompanha cada alteração e mostra no máximo as primeiras 200 linhas e 100 colunas, mas o arquivo inteiro é aberto. **Arquivo > Abrir > Abrir Arquivo de Texto** pergunta mesmo quando o nome do arquivo é incomum.

Para manter apenas os valores da planilha que você está vendo, clique em **Arquivo** e depois em **Salvar Como**, e escolha o tipo de arquivo **.csv** (ou **.tsv** e **.txt**, separados por tabulação). Antes de gravar o arquivo, o JeffCrunch informa o que a sua pasta de trabalho tem e o arquivo não conterá (as outras planilhas, fórmulas, imagens, gráficos, anotações e macros, por exemplo), pergunta como gravá-lo e se deve **Salvar Somente Valores**, **Salvar como .xlsx** para manter tudo, ou **Cancelar**:

- **Separador.** Vírgula, ponto e vírgula, tabulação, espaço, barra vertical ou um caractere que você digitar. Um **.csv** começa com ponto e vírgula onde o seu idioma escreve vírgula nos números, e com vírgula nos demais casos; um **.tsv** ou **.txt**, com tabulação.
- **Codificação do texto.** UTF-8 com marca (para que outros programas leiam corretamente as letras acentuadas), UTF-8 sem ela, UTF-16, ou uma página de código como Europa Ocidental (1252), as páginas ISO 8859, Mac Roman, ou as páginas japonesa, chinesa, coreana, hebraica, árabe, báltica, vietnamita e tailandesa. Uma letra para a qual a página de código não tem byte é gravada como ponto de interrogação, e a barra de status informa quantas foram.
- **Finais de linha.** Windows (retorno de carro e alimentação de linha), Linux e Mac (alimentação de linha), ou Mac antigo (retorno de carro).
- **Células.** **Como mostrado nas células** grava o que você vê. **Como armazenado** grava todos os dígitos de um número (0,1 e não 10%), e uma data como texto ISO, como 2026-03-04.

O texto vai entre aspas quando contém o separador, uma aspa, uma quebra de linha ou um espaço em qualquer extremidade. As outras planilhas, as fórmulas, os formatos e as configurações não ficam no arquivo, e a sua pasta de trabalho não é alterada ao gravar um.

Uma pasta de trabalho que veio de um arquivo de texto é gravada de volta no formato que o arquivo tinha: o mesmo separador, finais de linha e codificação. Se você a salvou como cópia sem alterar nenhuma célula, a cópia é o mesmo arquivo, byte por byte, e nada é perguntado.

## Salvar uma cópia como .xls ou .ods

Para criar uma cópia que programas mais antigos, ou programas que leem OpenDocument, possam abrir, clique em **Arquivo** e depois em **Salvar Como**, e escolha o tipo de arquivo **.xls** ou **.ods**. A cópia é feita a partir da sua pasta de trabalho como ela está agora, com as alterações que você ainda não salvou, e contém:

- os valores, com seus textos, números, valores lógicos e erros;
- as fórmulas, com os resultados que têm agora (uma fórmula de matriz continua sendo uma fórmula de matriz);
- os formatos de número, fontes, preenchimentos, bordas, alinhamento e se uma célula está bloqueada;
- células mescladas, larguras de coluna e alturas de linha, linhas e colunas ocultas, estrutura de tópicos de linhas e colunas e painéis congelados;
- as planilhas, na ordem, com seus nomes e quais estão ocultas, e os nomes que você definiu (e as áreas de impressão);
- os vínculos, e como cada planilha é impressa (o papel, as margens, a orientação, a escala, os cabeçalhos e rodapés e as quebras de página).

Um arquivo **.xls** também contém tabelas de dados e mantém as configurações de cálculo da pasta de trabalho. Uma cópia não consegue conter tudo. Antes de o JeffCrunch gravar o arquivo, ele informa o que a sua pasta de trabalho tem e o arquivo não terá: gráficos, imagens e outros desenhos, anotações, tabelas dinâmicas, tabelas, formatações condicionais, validação de dados, filtros, proteção de planilha, controles de formulário, macros e vínculos com outras pastas de trabalho (as fórmulas que os usam mantêm os valores que tinham). Uma fórmula que a forma antiga não consegue expressar (uma função que ela não tem, ou uma célula além da coluna IV ou da linha 65.536) é gravada como o valor a que chegou, e a mensagem informa quantas são. A forma antiga também para em 65.536 linhas, 256 colunas, 255 planilhas e 32.767 caracteres em uma célula, e em cerca de 4.000 formatos de célula e 56 cores: o que não couber é cortado, e a mensagem diz quanto. Você pode então escolher **Salvar Cópia**, **Salvar como .xlsx** para manter tudo, ou **Cancelar**.

A sua pasta de trabalho não é alterada pela cópia, e ela não é marcada como salva. O arquivo que você abriu nunca é gravado: dê outro nome à cópia. Uma cópia de um arquivo **.xls** ou **.ods** que você abriu pode ser salva no outro tipo.

## Salvar uma cópia

Clique em **Arquivo > Salvar uma Cópia** e escolha um nome. O JeffCrunch grava a pasta de trabalho nesse arquivo, com as alterações que você ainda não salvou, e você continua trabalhando na pasta de trabalho que tinha. O nome dela e as alterações não salvas permanecem como estavam. Você pode escolher qualquer tipo de arquivo que **Salvar Como** oferece. Uma cópia nunca é gravada sobre o próprio arquivo da pasta de trabalho.

## Manter uma cópia de backup

Clique em **Arquivo > Opções** e, na página **Geral**, ative **Sempre criar cópia de backup**. Cada vez que você salvar, o JeffCrunch primeiro copia o arquivo antigo para outro chamado "Backup de" seguido do nome dele, na mesma pasta. Se a cópia não puder ser feita, o aplicativo pergunta se deve salvar sem ela. A resposta é Não, a menos que você diga Sim, e o arquivo permanece como estava.

## Salvar como modelo

Um modelo é uma pasta de trabalho que você usa como ponto de partida. Ele pode conter fórmulas, estilos e planilhas. Clique em **Arquivo > Salvar como Modelo** e digite um nome. O modelo (um arquivo **.xltx**) vai para **Meus Modelos**, e a sua pasta de trabalho não é alterada. Em **Salvar Como**, você também pode escolher **Modelos (*.xltx)**, ou **Modelos de planilha OpenDocument (*.ots)**, para salvar um arquivo de modelo onde preferir.

Para iniciar uma pasta de trabalho a partir do seu modelo, clique em **Arquivo > Novo**. Os seus modelos aparecem em **Meus Modelos**. Clique em um deles. Uma nova pasta de trabalho é aberta com as células e planilhas do modelo. Ela ainda não tem arquivo próprio, então **Salvar** pede um nome, e o modelo não é alterado. Para remover um modelo da lista, clique com o botão direito sobre ele e escolha **Remover de Meus Modelos**.

## Outras formas de abrir um arquivo

Na página **Abrir**, clique em **Mais Formas de Abrir**. Você também pode clicar com o botão direito sobre um nome da lista Recentes.

- **Abrir Somente Leitura** abre a pasta de trabalho apenas para leitura. Você pode olhar, rolar, pesquisar e copiar, mas não pode alterar uma célula. A barra de título mostra "Somente Leitura". Se você pressionar **Salvar**, o JeffCrunch pede um novo nome, para que o arquivo aberto não seja alterado.
- **Abrir como Cópia** abre a pasta de trabalho sem arquivo próprio. **Salvar** pergunta onde guardá-la. O arquivo que você abriu nunca é alterado.
- **Abrir em Nova Janela** abre a pasta de trabalho em uma janela própria, mesmo quando a janela em que você está estiver vazia.
- **Abrir e Reparar** serve para um arquivo .xlsx ou .xls danificado. Veja abaixo.

## Abrir e Reparar

Às vezes um arquivo .xlsx é danificado. Um download que parou no meio, ou um disco que cometeu erros. Um arquivo .xlsx é um pacote de partes menores, como as planilhas, os estilos e o texto compartilhado. **Abrir e Reparar** lê cada parte que consegue. Uma parte que parou no meio é mantida até o ponto em que parou, então você pode obter as primeiras linhas de uma planilha. Uma parte que não pode ser lida é deixada de fora. Se a lista de partes ou as ligações entre elas tiverem sumido, o JeffCrunch as recria.

A pasta de trabalho reparada é aberta sem arquivo próprio, com um nome como "Orçamento reparado". Uma mensagem informa o que foi mantido até um corte e o que foi deixado de fora. O arquivo danificado não é alterado. Salve a pasta de trabalho reparada com um novo nome e confira os números antes de confiar neles.

### Um arquivo .xls

Um arquivo .xls também pode ser danificado. Se o seu invólucro externo estiver cortado, **Abrir e Reparar** procura no arquivo o início da pasta de trabalho e lê a partir dali. Uma planilha que foi cortada mantém as células anteriores ao corte. O que veio depois do corte é perdido, e você é informado disso. A pasta de trabalho reparada é aberta como cópia, então salve-a com um novo nome.

## Pastas de trabalho grandes

O JeffCrunch lê uma pasta de trabalho em segundo plano. As primeiras linhas da planilha que estava aberta quando a pasta foi salva aparecem quase de imediato, mesmo quando a pasta é muito grande. A barra de status, no canto inferior esquerdo, informa o que ele está fazendo: **Abrindo**, **Lendo** uma planilha com a contagem de células lidas até o momento, **Calculando** com uma porcentagem, e depois **Pronto**.

Você pode rolar, selecionar e copiar enquanto ele lê. As abas das outras planilhas são ativadas à medida que são lidas. Uma pasta de trabalho danificada é aberta até onde pode ser lida, e uma mensagem informa o que ficou de fora.

## Se um arquivo não abrir

Quando o JeffCrunch não consegue abrir um arquivo, ele diz o motivo em palavras simples e deixa as pastas de trabalho que você tem abertas como estão. Ele distingue um arquivo vazio de um que não é uma pasta de trabalho (um documento PDF, uma imagem, um documento de texto ou uma apresentação, por exemplo); um pacote zip cortado ou danificado, o que normalmente significa que um download ou uma cópia foi interrompido, e nesse caso copie-o novamente a partir do original; uma pasta de trabalho protegida por senha, que ele pede (veja [Abrir pastas de trabalho protegidas por senha](open-password)); e um arquivo composto que não contém nenhuma pasta de trabalho.

## Pastas de trabalho recentes

A página **Abrir**, na tela Arquivo, lista as pastas de trabalho que você abriu recentemente. Clique com o botão direito em uma delas para **fixá-la** no topo da lista ou removê-la da lista.

## Salvar

Pressione **{key:Save}**, ou clique em **Arquivo** e depois em **Salvar**, para salvar as suas alterações na pasta de trabalho que você abriu. Para manter o original e criar um novo arquivo, clique em **Arquivo** e depois em **Salvar Como**, e escolha um nome. Uma pasta de trabalho com macros é salva como arquivo **.xlsm**, um modelo como arquivo **.xltx** ou **.xltm**, e qualquer outra como arquivo **.xlsx**. Um nome que termine em outro tipo de pasta de trabalho não é aceito, e a mensagem informa qual terminação usar; os nomes terminados em **.xls**, **.ods**, **.csv**, **.tsv** e **.txt** criam cópias, como descrito acima.

O JeffCrunch salva apenas o que você alterou. Todo o restante do arquivo, como as outras planilhas, os gráficos, as imagens, as macros e as configurações que o JeffCrunch não mostra, é copiado como estava, byte por byte. Em uma planilha que você alterou, somente as células alteradas são gravadas novamente, junto com as fórmulas que dependem delas, que recebem os novos valores. Se você não alterou nenhuma célula, o arquivo fica intacto.

Se outro programa alterou o arquivo depois que você o abriu, o JeffCrunch não o sobrescreve sem perguntar, pois isso faria perder as alterações do outro programa. Ele pergunta se deve **Salvar uma Cópia**, **Substituir** o arquivo, ou **Cancelar**.

O novo arquivo é gravado primeiro ao lado do antigo, e só ocupa o lugar dele quando estiver completo. Se algo der errado, você vê uma mensagem, e o arquivo que estava ali permanece como estava.

Se as fórmulas ainda estiverem sendo calculadas quando você salvar, o JeffCrunch espera que terminem, para que o arquivo contenha seus resultados. Se você pressionar **Esc** para interromper o cálculo nesse meio-tempo, o salvamento é cancelado, e a barra de status informa isso; salve novamente quando quiser. O JeffCrunch deixa uma coisa de fora do arquivo: a lista da ordem em que um programa calculou as fórmulas pela última vez (a cadeia de cálculo). Um programa que abrir a pasta de trabalho em seguida a recria. Se o JeffCrunch não conseguiu calcular alguma fórmula, ou se você interrompeu o cálculo, o arquivo pede que o programa calcule novamente todas as fórmulas ao abrir a pasta de trabalho.

## Fechar com alterações

Se você fechar uma pasta de trabalho que tem alterações não salvas, o JeffCrunch pergunta se deve **Salvar** essas alterações, **Não Salvar** ou **Cancelar** e continuar trabalhando. Um asterisco (*) no título da janela indica que há alterações não salvas.

## Modelos prontos

Em **Arquivo > Novo**, abaixo da pasta de trabalho em branco, há pastas de trabalho prontas, agrupadas em: **Finanças** (um orçamento mensal, uma planilha de empréstimo e um registro de talão de cheques), **Empresa** (uma fatura, um relatório de despesas, uma folha de horas, uma lista de estoque, um registro de quilometragem e um relatório de vendas), **Planejamento** (um calendário e um plano de projeto) e **Educação** (um diário de notas). Cada um mostra uma pequena imagem do seu topo. Clique em um deles para iniciar uma nova pasta de trabalho a partir dele. Digite nas caixas amarelas. Os demais números são calculados para você. A pasta de trabalho não tem arquivo próprio até que você a salve.
