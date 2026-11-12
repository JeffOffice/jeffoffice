# Abrir e salvar pastas de trabalho
<!-- keywords: recuperar trabalho não salvo, recuperação automática, travamento, falha, cópia, formato antigo, opendocument, compatibilidade, nova, em branco, Pasta1, sem título, abrir, salvar, salvar como, arquivo, pasta de trabalho, xlsx, xlsm, xls, ods, csv, tsv, arquivo de texto, delimitado, importar, exportar, recentes, fixar, arrastar, soltar, carregamento, grande, macros, não salvo, alterações, fechar, cópia, cadeia de cálculo, modelo, xltx, xltm, não é possível abrir, danificado, criptografado, senha, salvar uma cópia, somente leitura, abrir como cópia, nova janela, cópia de backup, meus modelos, salvar como modelo, ots, reparar, abrir e reparar, modelos prontos, orçamento, fatura, folha de horas, empréstimo, calendário, diário de notas, largura fixa, colunas fixas, separador decimal, separador de milhares, sinal de menos à direita, html, página da web, htm, tabelas de uma página -->

Quando o JeffCrunch é iniciado sem arquivo para abrir, a janela mostra uma pasta de trabalho nova e vazia, com uma planilha chamada Pasta1. Cada nova pasta de trabalho recebe o próximo número (Pasta2, Pasta3, e assim por diante), e um número não é usado de novo depois que sua janela é fechada. Para iniciar outra, pressione **{key:New}**, ou clique em **Arquivo** e depois em **Novo**, e escolha **Pasta de Trabalho em Branco**; ela abre em uma janela própria. Uma nova pasta de trabalho que você não alterou dá lugar quando você abre um arquivo na sua janela; uma que você alterou permanece, e o arquivo abre em outra janela. **Salvar** em uma nova pasta de trabalho pede um nome, assim como **Salvar Como**, e a salva como um arquivo .xlsx. Fechar uma nova pasta de trabalho que você não alterou não pede nada.

Para abrir uma pasta de trabalho, pressione **{key:Open}**, ou clique em **Arquivo** e depois em **Abrir**, e escolha **Procurar**. Você também pode arrastar uma pasta de trabalho do gerenciador de arquivos para a janela. Uma pasta de trabalho que já está aberta em uma janela não é aberta uma segunda vez: essa janela passa para a frente.

O JeffCrunch abre pastas de trabalho **.xlsx** e pastas de trabalho **.xlsm** (as que contêm macros), além dos modelos **.xltx** e **.xltm** (um modelo que contém macros). Ele não executa macros e as mantém intactas: o projeto de macros, sua assinatura e as partes que os referenciam são gravados de volta exatamente como estavam. Um modelo é salvo como modelo, e uma pasta de trabalho com macros como uma pasta de trabalho com macros, pois o tipo faz parte do arquivo.

## Recuperar trabalho não salvo

Se o JeffCrunch fechar inesperadamente, como em uma queda de energia, na próxima inicialização aparece uma lista das cópias que ele guardou do seu trabalho não salvo. Escolha quais abrir e clique em **Abrir**; cada uma abre como uma nova pasta de trabalho que ainda não tem arquivo, então clique em **Salvar** e escolha um nome. **Excluir** descarta as cópias marcadas, e **Agora Não** as deixa para depois. Você pode pedir a lista a qualquer momento com **Arquivo > Abrir > Recuperar Trabalho Não Salvo**. Veja [Alterar as configurações do JeffCrunch](options) para saber com que frequência as cópias são feitas.

## Outros tipos de arquivo

O JeffCrunch também abre pastas de trabalho **.xls** mais antigas, planilhas OpenDocument (**.ods** e a forma plana **.fods**) e arquivos de texto com valores (**.csv**, **.tsv** e **.txt**). Uma pasta de trabalho aberta a partir de um deles é salva como um novo arquivo **.xlsx** (ou como uma cópia **.xls** ou **.ods**: veja abaixo). Ao pressionar **{key:Save}**, o JeffCrunch avisa isso e pede um nome, e o arquivo que você abriu nunca é alterado. As imagens, formas, caixas de texto, grupos, linhas e Texto Artístico de um arquivo **.ods** vão para o novo arquivo com suas posições, tamanhos, rotações, ordem, nomes, texto alternativo, preenchimentos e contornos; os gráficos vão junto, desenhados a partir das células que leem.

Uma planilha OpenDocument é lida com suas planilhas, valores, fórmulas (com os resultados que o arquivo guardou), formatos de número, fontes, preenchimentos, bordas, células mescladas, larguras de coluna e alturas de linha, linhas e colunas ocultas, painéis congelados e nomes. Gráficos, imagens e outros desenhos, comentários, tabelas dinâmicas, macros, formatações condicionais e validação de dados ainda não são lidos; uma mensagem no topo informa quais deles o arquivo tem, e uma cópia salva daqui não os terá. Uma planilha protegida por senha pede a senha ao ser aberta: veja [Abrir pastas de trabalho protegidas por senha](open-password).

Um arquivo de texto é uma única planilha. O JeffCrunch descobre como os valores são separados (por vírgulas, ponto e vírgula, tabulações ou barras verticais) e como as letras são codificadas (UTF-8 ou UTF-16, ou então a página de código do seu idioma), e lê cada valor como você o digitaria no seu idioma: um número como 1.234,5 em alemão, uma porcentagem, um valor monetário, uma data com ano (como 15/03/2024, ou 15.03.2024 em alemão), uma hora, VERDADEIRO ou FALSO (no seu idioma, como WAHR e FALSCH em alemão, e TRUE e FALSE em inglês). Um valor que começa com sinal de igual é uma fórmula. Números que começam com zero (como 007) e números de mais de 15 dígitos permanecem como texto, para que um CEP ou um número de conta não seja alterado; o mesmo vale para um dia e um mês sem ano, em qualquer ordem e em qualquer idioma (como 3/4, 12,5, 1-2, 1 de março ou 25 dez), pois o ano teria de ser inventado, e para textos como o nome do gene MARCH1. Uma planilha tem 1.048.576 linhas e 16.384 colunas; se o arquivo tiver mais, o JeffCrunch abre o que cabe e informa isso.

Ao abrir um arquivo de texto, uma janela mostra primeiro as suas primeiras linhas e pergunta como ele deve ser lido: o que separa os campos (vírgula, ponto e vírgula, tabulação, espaço, barra vertical ou um caractere que você digitar, com a opção de contar vários separadores seguidos como um só), o que delimita um campo que contém um separador, a codificação do texto (Unicode, as páginas de código do Windows, as páginas ISO 8859, Mac Roman e as do Leste Asiático), e a linha em que começar. Acima de cada coluna você escolhe como ela é lida: **Como digitado**, **Texto** (para manter um código como 007 ou um número de conta longo do jeito que está), uma **Data** na ordem que você indicar, ou **Ignorar**. Alguns arquivos alinham seus valores em colunas de mesma largura, com espaços entre elas, e sem separador algum. Marque **As colunas têm largura fixa**, e as primeiras linhas do arquivo aparecem em uma caixa. Clique entre dois caracteres para iniciar uma coluna ali, e clique de novo para removê-la; ou digite as posições em **Colunas começam em**, como 10, 25, 40. Cada parte é cortada nessas posições, e os espaços ao redor são retirados. **Separador decimal** e **Separador de milhares** indicam ao JeffCrunch como os números deste arquivo são escritos, qualquer que seja o idioma do seu programa, de modo que 1.234,50 seja mil duzentos e trinta e quatro vírgula cinquenta quando o arquivo usa vírgula como separador decimal. Marque **Um sinal de menos pode vir depois do número** para arquivos que escrevem -12 como 12-. A prévia acompanha cada alteração e mostra no máximo as primeiras 200 linhas e 100 colunas, mas o arquivo inteiro é aberto. **Arquivo > Abrir > Abrir Arquivo de Texto** pergunta mesmo quando o nome do arquivo é incomum.

O JeffCrunch também abre **páginas da web** (**.html** e **.htm**), e uma página salva com o nome de uma pasta de trabalho. Cada tabela da página se torna uma planilha, chamada **Tabela1**, **Tabela2** e assim por diante. As células são lidas como se você as tivesse digitado: um número é um número, e uma data é uma data. Uma célula que está mesclada em várias colunas ou linhas deixa células vazias, de modo que as demais células permaneçam em suas colunas. As palavras fora das tabelas, as imagens e a aparência da página não são lidas, e uma página sem tabela não é aberta. Nada que a página mencione é baixado da internet.

Para manter apenas os valores da planilha que você está vendo, clique em **Arquivo**, depois em **Salvar Como**, e escolha o tipo **.csv** (ou **.tsv** e **.txt**, separados por tabulação). Antes de gravar o arquivo, o JeffCrunch informa o que a sua pasta de trabalho tem e que o arquivo não conterá (as outras planilhas, fórmulas, imagens, gráficos, anotações e macros, por exemplo), pergunta como gravá-lo e se deve **Salvar Somente Valores**, **Salvar como .xlsx** para manter tudo, ou **Cancelar**:

- **Separador.** Vírgula, ponto e vírgula, tabulação, espaço, barra vertical ou um caractere que você digitar. Um **.csv** começa com ponto e vírgula onde o seu idioma escreve vírgula nos números, e com vírgula nos demais casos; um **.tsv** ou **.txt**, com tabulação.
- **Codificação do texto.** UTF-8 com marca (para que outros programas reconheçam corretamente as letras acentuadas), UTF-8 sem marca, UTF-16, ou uma página de código como Europa Ocidental (1252), as páginas ISO 8859, Mac Roman, ou as páginas japonesa, chinesa, coreana, hebraica, árabe, báltica, vietnamita e tailandesa. Uma letra para a qual a página de código não tem byte é gravada como ponto de interrogação, e a barra de status informa quantas foram.
- **Finais de linha.** Windows (retorno de carro e alimentação de linha), Linux e Mac (alimentação de linha), ou Mac antigo (retorno de carro).
- **Células.** **Como mostrado nas células** grava o que você vê. **Como armazenado** grava todos os dígitos de um número (0,1 e não 10%), e uma data como texto ISO, como 2026-03-04.

O texto é colocado entre aspas quando contém o separador, uma aspa, uma quebra de linha ou um espaço em qualquer extremidade. As outras planilhas, as fórmulas, os formatos e as configurações não ficam no arquivo, e a sua pasta de trabalho não é alterada ao gravar um deles.

Uma pasta de trabalho que veio de um arquivo de texto é gravada de volta no formato que o arquivo tinha: o mesmo separador, finais de linha e codificação. Se você a salvou como cópia sem alterar nenhuma célula, a cópia é o mesmo arquivo, byte por byte, e nada é perguntado.

## Salvar uma cópia como .xls ou .ods

Para fazer uma cópia que programas mais antigos, ou programas que leem OpenDocument, possam abrir, clique em **Arquivo**, depois em **Salvar Como**, e escolha o tipo **.xls** ou **.ods**. A cópia é feita a partir da sua pasta de trabalho como está agora, com as alterações que você ainda não salvou, e contém:

- os valores, com seus textos, números, valores lógicos e erros;
- as fórmulas, com os resultados que elas têm agora (uma fórmula de matriz continua sendo uma fórmula de matriz);
- os formatos de número, fontes, preenchimentos, bordas, alinhamento e se a célula está bloqueada;
- células mescladas, larguras de coluna e alturas de linha, linhas e colunas ocultas, níveis de tópicos de linhas e colunas, e painéis congelados;
- as planilhas, na ordem, com seus nomes e quais estão ocultas, os nomes que você definiu (e as áreas de impressão);
- os gráficos de cada planilha (somente em uma cópia **.ods**): colunas, barras, linhas, áreas, pizzas, roscas e dispersão, com suas séries e as células que leem, categorias, títulos, legenda, eixos (títulos, escala, formato de número), rótulos de dados, cores e posição (o que um gráfico **.ods** não tem como representar, como linhas de tendência, barras de erro e eixos secundários, é informado na mensagem);
- as imagens, formas, caixas de texto, grupos, linhas e Texto Artístico de cada planilha (somente em uma cópia **.ods**), com suas posições, tamanhos, rotações, ordem, nomes, texto alternativo, preenchimentos, contornos, textos, e as próprias imagens com seus recortes (o Texto Artístico é salvo como a forma de fontwork que o OpenDocument possui);
- os links, e a forma como cada planilha é impressa (papel, margens, orientação, escala, cabeçalhos e rodapés, e quebras de página).

Um arquivo **.xls** também guarda tabelas de dados e mantém as configurações de cálculo da pasta de trabalho. Uma cópia não pode conter tudo. Antes de o JeffCrunch gravar o arquivo, ele informa o que a sua pasta de trabalho tem e que o arquivo não terá: gráficos, imagens e outros desenhos (em uma cópia **.xls**; uma cópia **.ods** mantém-nos, inclusive os gráficos, exceto diagramas), anotações, tabelas dinâmicas, tabelas, formatações condicionais, validação de dados, filtros, proteção de planilha, controles de formulário, macros e links para outras pastas de trabalho (as fórmulas que os usam mantêm os valores que tinham). Uma fórmula que a forma mais antiga não consegue expressar (uma função que ela não tem, ou uma célula além da coluna IV ou da linha 65.536) é gravada como o valor a que chegou, e a mensagem informa quantas são. A forma mais antiga também se limita a 65.536 linhas, 256 colunas, 255 planilhas e 32.767 caracteres em uma célula, e a cerca de 4.000 formatos de célula e 56 cores: o que não couber é cortado, e a mensagem informa quanto. Você pode então escolher **Salvar Cópia**, **Salvar como .xlsx** para manter tudo, ou **Cancelar**.

A sua pasta de trabalho não é alterada pela cópia, e ela não é marcada como salva. O arquivo que você abriu nunca é gravado: dê à cópia outro nome. Uma cópia de um arquivo **.xls** ou **.ods** que você abriu pode ser salva no outro tipo.

## Salvar uma cópia

Clique em **Arquivo > Salvar uma Cópia** e escolha um nome. O JeffCrunch grava a pasta de trabalho nesse arquivo, com as alterações que você ainda não salvou, e você continua trabalhando na pasta de trabalho que tinha. O nome dela e as alterações não salvas permanecem como estavam. Você pode escolher qualquer tipo de arquivo que **Salvar Como** oferece. Uma cópia nunca é gravada sobre o arquivo próprio da pasta de trabalho.

## Manter uma cópia de backup

Clique em **Arquivo > Opções** e, na página **Geral**, ative **Sempre criar cópia de backup**. Cada vez que você salvar, o JeffCrunch primeiro copia o arquivo antigo para outro chamado "Backup de" seguido do nome dele, na mesma pasta. Se a cópia não puder ser feita, o aplicativo pergunta se deve salvar sem ela. A resposta é Não, a menos que você diga Sim, e o arquivo permanece como estava.

## Salvar como modelo

Um modelo é uma pasta de trabalho que você usa como ponto de partida. Ele pode conter fórmulas, estilos e planilhas. Clique em **Arquivo > Salvar como Modelo** e digite um nome. O modelo (um arquivo **.xltx**) vai para **Meus Modelos**, e a sua pasta de trabalho não é alterada. Em **Salvar Como**, você também pode escolher **Modelos (*.xltx)**, ou **Modelos de planilha OpenDocument (*.ots)**, para salvar um arquivo de modelo onde preferir.

Para iniciar uma pasta de trabalho a partir do seu modelo, clique em **Arquivo > Novo**. Os seus modelos aparecem em **Meus Modelos**. Clique em um deles. Uma nova pasta de trabalho abre com as células e planilhas do modelo. Ela ainda não tem arquivo próprio, então **Salvar** pede um nome, e o modelo não é alterado. Para remover um modelo da lista, clique com o botão direito sobre ele e escolha **Remover de Meus Modelos**.

## Outras formas de abrir um arquivo

Na página **Abrir**, clique em **Mais Formas de Abrir**. Você também pode clicar com o botão direito sobre um nome na lista de Recentes.

- **Abrir Somente Leitura** abre a pasta de trabalho apenas para leitura. Você pode consultar, rolar, pesquisar e copiar, mas não pode alterar uma célula. A barra de título mostra "Somente Leitura". Se pressionar **Salvar**, o JeffCrunch pede um novo nome, para que o arquivo que você abriu não seja alterado.
- **Abrir como Cópia** abre a pasta de trabalho sem arquivo próprio. **Salvar** pergunta onde guardá-la. O arquivo que você abriu nunca é alterado.
- **Abrir em Nova Janela** abre a pasta de trabalho em uma janela própria, mesmo quando a janela em que você está estiver vazia.
- **Abrir e Reparar** serve para um arquivo .xlsx ou .xls danificado. Veja abaixo.

## Abrir e Reparar

Às vezes um arquivo .xlsx está danificado. Um download foi interrompido no meio, ou um disco cometeu erros. Um arquivo .xlsx é um conjunto de partes menores, como as planilhas, os estilos e o texto compartilhado. **Abrir e Reparar** lê cada parte que consegue. Uma parte interrompida no meio é mantida até o ponto onde parou, então você pode obter as primeiras linhas de uma planilha. Uma parte que não pode ser lida é deixada de fora. Se a lista de partes ou as ligações entre elas estiverem perdidas, o JeffCrunch as refaz.

A pasta de trabalho reparada abre sem arquivo próprio, com um nome como "Orçamento reparado". Uma mensagem informa o que foi mantido até uma interrupção e o que foi deixado de fora. O arquivo danificado não é alterado. Salve a pasta de trabalho reparada com um novo nome e confira os números antes de confiar neles.

### Um arquivo .xls

Um arquivo .xls também pode estar danificado. Se o seu invólucro externo estiver cortado, **Abrir e Reparar** procura no arquivo o início da pasta de trabalho e lê a partir dali. Uma planilha que foi cortada mantém as células anteriores ao corte. O que veio depois do corte se perde, e você é informado disso. A pasta de trabalho reparada abre como cópia, então salve-a com um novo nome.

## Pastas de trabalho grandes

O JeffCrunch lê uma pasta de trabalho em segundo plano. As primeiras linhas da planilha que estava aberta quando a pasta de trabalho foi salva aparecem quase de imediato, mesmo quando a pasta é muito grande. A barra de status, no canto inferior esquerdo, informa o que ele está fazendo: **Abrindo**, **Lendo** uma planilha com a contagem de células lidas até então, **Calculando** com uma porcentagem, e depois **Pronto**.

Você pode rolar, selecionar e copiar enquanto ele lê. As abas das outras planilhas são ativadas à medida que são lidas. Uma pasta de trabalho danificada é aberta até onde pode ser lida, e uma mensagem informa o que ficou de fora.

## Se um arquivo não abrir

Quando o JeffCrunch não consegue abrir um arquivo, ele informa o motivo em palavras simples e deixa as pastas de trabalho que você tem abertas como estão. Ele distingue um arquivo vazio de um que não é uma pasta de trabalho (um documento PDF, uma imagem, um documento de texto ou uma apresentação, por exemplo); um pacote zip cortado ou danificado, o que geralmente significa que um download ou uma cópia foi interrompido, de modo que você deve copiá-lo novamente a partir do original; uma pasta de trabalho protegida por senha, que ele pede (veja [Abrir pastas de trabalho protegidas por senha](open-password)); e um arquivo composto que não contém nenhuma pasta de trabalho.

## Pastas de trabalho recentes

A página **Abrir** da tela Arquivo lista as pastas de trabalho que você abriu recentemente. Clique com o botão direito em uma delas para **fixá-la** no topo da lista ou removê-la da lista.

## Salvar

Pressione **{key:Save}**, ou clique em **Arquivo** e depois em **Salvar**, para salvar as suas alterações na pasta de trabalho que você abriu. Para manter o original e criar um novo arquivo, clique em **Arquivo** e depois em **Salvar Como**, e escolha um nome. Uma pasta de trabalho com macros é salva como arquivo **.xlsm**, um modelo como arquivo **.xltx** ou **.xltm**, e qualquer outra como arquivo **.xlsx**. Um nome que termina em outro tipo de pasta de trabalho não é aceito, e a mensagem informa qual terminação usar; os nomes terminados em **.xls**, **.ods**, **.csv**, **.tsv** e **.txt** criam cópias, como descrito acima.

O JeffCrunch salva apenas o que você alterou. Todo o resto do arquivo, como as outras planilhas, os gráficos, as imagens, as macros e as configurações que o JeffCrunch não mostra, é copiado como estava, byte por byte. Em uma planilha alterada, somente as células que você alterou são gravadas novamente, junto com as fórmulas que dependem delas, que recebem os novos valores. Se você não alterou nenhuma célula, o arquivo é deixado como está.

Se outro programa alterou o arquivo depois que você o abriu, o JeffCrunch não o sobrescreve sem perguntar, porque isso apagaria as alterações do outro programa. Ele pergunta se deve **Salvar uma Cópia**, **Substituir** o arquivo, ou **Cancelar**.

O novo arquivo é gravado primeiro ao lado do antigo, e só ocupa o lugar dele quando estiver completo. Se algo der errado, você vê uma mensagem, e o arquivo que estava ali permanece como era.

Se as fórmulas ainda estiverem sendo calculadas quando você salvar, o JeffCrunch espera que terminem, para que o arquivo contenha os resultados. Se você pressionar **Esc** para interromper o cálculo nesse meio-tempo, o salvamento é cancelado, e a barra de status informa isso; salve de novo quando quiser. O JeffCrunch deixa uma coisa de fora do arquivo: a lista da ordem em que um programa calculou as fórmulas por último (a cadeia de cálculo). Um programa que abrir a pasta de trabalho depois a refaz. Se o JeffCrunch não conseguiu calcular alguma fórmula, ou se você interrompeu o cálculo, o arquivo pede a esse programa que calcule todas as fórmulas novamente ao abrir a pasta de trabalho.

## Fechar com alterações

Se você fechar uma pasta de trabalho que tem alterações não salvas, o JeffCrunch pergunta se deve **Salvar** essas alterações, **Não Salvar**, ou **Cancelar** e continuar trabalhando. Um asterisco (*) no título da janela indica que há alterações não salvas.

## Modelos prontos

Em **Arquivo > Novo**, abaixo da pasta de trabalho em branco, há pastas de trabalho prontas, organizadas em grupos: **Finanças** (um orçamento mensal, um cronograma de empréstimo e um registro de talão de cheques), **Negócios** (uma fatura, um relatório de despesas, uma folha de horas, uma lista de estoque, um registro de quilometragem e um relatório de vendas), **Planejamento** (um calendário e um plano de projeto) e **Educação** (um diário de notas). Cada uma mostra uma pequena imagem do seu topo. Clique em uma para iniciar uma nova pasta de trabalho a partir dela. Digite nas caixas amarelas. Os demais números são calculados para você. A pasta de trabalho não tem arquivo próprio até que você a salve.
