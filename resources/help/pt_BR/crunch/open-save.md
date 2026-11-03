# Abrir e salvar pastas de trabalho
<!-- keywords: cópia, formato antigo, opendocument, compatibilidade, novo, nova, em branco, pasta1, sem título, abrir, salvar, salvar como, arquivo, pasta de trabalho, planilha, xlsx, xlsm, xls, ods, csv, tsv, arquivo de texto, delimitado, separado por vírgulas, importar, exportar, recentes, fixado, arrastar, soltar, carregando, grande, macros, não salvo, alterações, fechar, cadeia de cálculo, modelo, xltx, xltm, não abre, danificado, corrompido, criptografado, senha -->

Quando o JeffCrunch é iniciado sem nenhum arquivo para abrir, a janela mostra uma pasta de trabalho nova e vazia, com uma planilha, chamada Pasta1. Cada nova pasta de trabalho recebe o número seguinte (Pasta2, Pasta3 e assim por diante), e um número não é reutilizado depois que sua janela é fechada. Para começar outra, pressione **{key:New}** ou clique em **Arquivo**, em **Novo** e escolha **Pasta de trabalho em branco**; ela abre em uma janela própria. Uma pasta de trabalho nova que você não alterou dá lugar ao arquivo que você abrir na janela dela; uma que você alterou permanece, e o arquivo abre em outra janela. **Salvar** em uma pasta de trabalho nova pede um nome, como **Salvar Como**, e a salva como arquivo .xlsx. Fechar uma pasta de trabalho nova que você não alterou não pede nenhuma confirmação.

Para abrir uma pasta de trabalho, pressione **{key:Open}** ou clique em **Arquivo** e em **Abrir** e escolha **Procurar**. Você também pode arrastar uma pasta de trabalho do gerenciador de arquivos para a janela. Uma pasta de trabalho que já está aberta em uma janela não é aberta uma segunda vez: essa janela passa para a frente.

O JeffCrunch abre pastas de trabalho **.xlsx** e **.xlsm** (as que contêm macros) e os modelos **.xltx** e **.xltm** (modelo que contém macros). Ele não executa macros e as mantém intactas: o projeto de macro, sua assinatura e as partes que as nomeiam são gravados de volta exatamente como estavam. Um modelo é salvo como modelo, e uma pasta de trabalho com macros como pasta de trabalho com macros, porque o tipo faz parte do arquivo.

## Outros tipos de arquivo

O JeffCrunch também abre pastas de trabalho **.xls** antigas, planilhas OpenDocument (**.ods** e o formato simples **.fods**) e arquivos de texto de valores (**.csv**, **.tsv** e **.txt**). Uma pasta de trabalho aberta a partir de um deles é salva como um novo arquivo **.xlsx** (ou como uma cópia **.xls** ou **.ods**: veja abaixo): quando você pressiona **{key:Save}**, o JeffCrunch avisa e pede um nome, e o arquivo que você abriu nunca é alterado.

Uma planilha OpenDocument é lida com suas planilhas, valores, fórmulas (com os resultados que o arquivo guardou), formatos de número, fontes, preenchimentos, bordas, células mescladas, larguras de coluna e alturas de linha, linhas e colunas ocultas, painéis congelados e nomes. Gráficos, imagens e outros desenhos, comentários, tabelas dinâmicas, macros, formatações condicionais e validação de dados ainda não são lidos; uma mensagem na parte superior informa quais deles o arquivo tem, e uma cópia salva a partir daqui não os terá. Uma planilha protegida por senha pede a senha quando você a abre: veja [Abrir pastas de trabalho protegidas por senha](open-password).

Um arquivo de texto é uma planilha. O JeffCrunch descobre como os valores estão separados (por vírgulas, ponto e vírgula, tabulações ou barras verticais) e como as letras estão codificadas (UTF-8 ou UTF-16, ou então a página de código do seu idioma), e lê cada valor como você o digitaria no seu idioma: um número como 1.234,5, uma porcentagem, um valor em dinheiro, uma data com o ano (como 15/03/2024, ou 15.03.2024 em alemão), uma hora, VERDADEIRO ou FALSO (no seu idioma, como WAHR e FALSCH em alemão, e em inglês). Um valor que começa com sinal de igual é uma fórmula. Dígitos que começam com zero (como 007) e números com mais de 15 dígitos continuam como texto, para que um CEP ou um número de conta não seja alterado, e o mesmo acontece com um dia e um mês sem ano, em qualquer ordem e em qualquer idioma (como 3/4, 12.5, 1-2, 1º de março ou 25 dez), porque o ano teria de ser inventado, e com textos como o nome de gene MARCH1. Uma planilha comporta 1.048.576 linhas e 16.384 colunas; se o arquivo tiver mais, o JeffCrunch abre o que couber e avisa.

Quando você abre um arquivo de texto, uma janela mostra primeiro as primeiras linhas e pergunta como ele deve ser lido: o que separa os campos (vírgula, ponto e vírgula, tabulação, espaço, barra vertical ou um caractere que você digitar, em qualquer combinação, com separadores consecutivos contados como um só, se quiser), o que delimita um campo que contém um separador, a codificação de texto (Unicode, as páginas de código do Windows, as páginas ISO 8859, Mac Roman e as do Leste Asiático) e a linha em que começar. Acima de cada coluna, você escolhe como ela é lida: **Como digitado**, **Texto** (para manter um código de peça como 007 ou um número de conta longo como está), **Data** na ordem que você indicar, ou **Ignorar**. A visualização acompanha cada alteração e mostra no máximo as primeiras 200 linhas e 100 colunas, mas o arquivo inteiro é aberto. **Arquivo > Abrir > Abrir um arquivo de texto** faz essas perguntas mesmo quando o nome do arquivo é incomum.

Para manter somente os valores da planilha que você está vendo, clique em **Arquivo**, em **Salvar Como** e escolha o tipo de arquivo **.csv** (ou **.tsv** e **.txt**, separados por tabulação). Antes de gravar o arquivo, o JeffCrunch informa o que a sua pasta de trabalho tem e o arquivo não comportará (as outras planilhas, fórmulas, imagens, gráficos, anotações e macros, por exemplo), pergunta como gravá-lo e se você deseja **Salvar Somente Valores**, **Salvar como .xlsx…** e manter tudo, ou **Cancelar**:

- **Separador.** Vírgula, ponto e vírgula, tabulação, espaço, barra vertical ou um caractere que você digitar. Um **.csv** começa com ponto e vírgula quando o seu idioma usa vírgula nos números (como no português do Brasil), e com vírgula nos demais casos; um **.tsv** ou **.txt**, com tabulação.
- **Codificação de texto.** UTF-8 com marca (para que outros programas leiam corretamente as letras acentuadas), UTF-8 sem marca, UTF-16 ou uma página de código como Europa Ocidental (1252), as páginas ISO 8859 ou Mac Roman. Uma letra para a qual a página de código não tem byte é gravada como ponto de interrogação, e a barra de status informa quantas foram.
- **Finais de linha.** Windows (retorno de carro e alimentação de linha), Linux e Mac (alimentação de linha) ou Mac antigo (retorno de carro).
- **Células.** **Como mostrado nas células** grava o que você vê. **Como armazenado** grava todos os dígitos de um número (0,1 e não 10%) e uma data como texto ISO, por exemplo 2026-03-04.

O texto fica entre aspas quando contém o separador, aspas, uma quebra de linha ou um espaço em uma das extremidades. As outras planilhas, as fórmulas, os formatos e as configurações não vão para o arquivo, e gravá-lo não altera a sua pasta de trabalho.

Uma pasta de trabalho que veio de um arquivo de texto é gravada de volta no formato que o arquivo tinha: o mesmo separador, os mesmos finais de linha e a mesma codificação. Se você a salvou como cópia sem alterar nenhuma célula, a cópia é o mesmo arquivo, byte por byte, e nada é perguntado.

## Salvar uma cópia como .xls ou .ods

Para criar uma cópia que programas mais antigos, ou programas que leem OpenDocument, possam abrir, clique em **Arquivo**, em **Salvar Como** e escolha o tipo de arquivo **.xls** ou **.ods**. A cópia é feita a partir da sua pasta de trabalho como está agora, com as alterações ainda não salvas, e contém:

- os valores, com seus textos, números, valores lógicos e erros;
- as fórmulas, com os resultados atuais (uma fórmula de matriz continua sendo uma fórmula de matriz);
- os formatos de número, fontes, preenchimentos, bordas, alinhamento e se a célula está bloqueada;
- células mescladas, larguras de coluna e alturas de linha, linhas e colunas ocultas, estruturas de tópicos de linhas e colunas e painéis congelados;
- as planilhas, na ordem, com seus nomes e quais estão ocultas, e os nomes que você definiu (e as áreas de impressão);
- os links e a forma como cada planilha é impressa (papel, margens, orientação, escala, cabeçalhos e rodapés e quebras de página).

Um arquivo **.xls** também contém tabelas de dados e mantém as configurações de cálculo da pasta de trabalho. Uma cópia não comporta tudo. Antes de gravar o arquivo, o JeffCrunch informa o que a sua pasta de trabalho tem e o arquivo não terá: gráficos, imagens e outros desenhos, anotações, tabelas dinâmicas, tabelas, formatações condicionais, validação de dados, filtros, proteção de planilha, controles de formulário, macros e links para outras pastas de trabalho (as fórmulas que os usam mantêm os valores que tinham). Uma fórmula que o formato antigo não consegue expressar (uma função que ele não tem, ou uma célula além da coluna IV ou da linha 65.536) é gravada como o valor resultante, e a mensagem informa quantas foram. O formato antigo também se limita a 65.536 linhas, 256 colunas, 255 planilhas e 32.767 caracteres por célula, e a cerca de 4.000 formatos de célula e 56 cores: o que não couber é cortado, e a mensagem informa quanto. Você pode então escolher **Salvar Cópia**, **Salvar como .xlsx…** e manter tudo, ou **Cancelar**.

A cópia não altera a sua pasta de trabalho, que também não é marcada como salva. O arquivo que você abriu nunca é gravado: dê outro nome à cópia. Uma cópia de um arquivo **.xls** ou **.ods** que você abriu pode ser salva no outro tipo.

## Pastas de trabalho grandes

O JeffCrunch lê uma pasta de trabalho em segundo plano. As primeiras linhas da planilha que estava aberta quando a pasta de trabalho foi salva aparecem quase imediatamente, mesmo quando a pasta de trabalho é muito grande. A barra de status, no canto inferior esquerdo, informa o que ele está fazendo: **Abrindo**, **Lendo** uma planilha com a contagem de células lidas até o momento, **Calculando** com uma porcentagem e, por fim, **Pronto**.

Você pode rolar, selecionar e copiar enquanto ele lê. As guias das outras planilhas são ativadas quando elas são lidas. Uma pasta de trabalho danificada é aberta até onde for possível lê-la, e uma mensagem informa o que ficou de fora.

## Se um arquivo não abrir

Quando o JeffCrunch não consegue abrir um arquivo, ele explica o motivo em palavras simples e deixa como estão as pastas de trabalho que você tem abertas. Ele distingue um arquivo vazio de um que não é pasta de trabalho (um documento PDF, uma imagem, um documento de texto ou uma apresentação, por exemplo); um pacote zip truncado ou danificado, o que geralmente significa que um download ou uma cópia foi interrompido, então copie-o novamente do original; uma pasta de trabalho protegida por senha, que ele pede (veja [Abrir pastas de trabalho protegidas por senha](open-password)); e um arquivo composto que não contém pasta de trabalho.

## Pastas de trabalho recentes

A página **Abrir** da tela Arquivo lista as pastas de trabalho que você abriu recentemente. Clique com o botão direito em uma delas para **fixá-la** no topo da lista ou para removê-la da lista.

## Salvar

Pressione **{key:Save}** ou clique em **Arquivo** e em **Salvar** para salvar as alterações na pasta de trabalho que você abriu. Para manter o original e criar um novo arquivo, clique em **Arquivo** e em **Salvar Como** e escolha um nome. Uma pasta de trabalho com macros é salva como arquivo **.xlsm**, um modelo como arquivo **.xltx** ou **.xltm**, e qualquer outra como arquivo **.xlsx**. Um nome que termine com a extensão de outro tipo de pasta de trabalho não é aceito, e a mensagem informa qual extensão usar; os nomes terminados em **.xls**, **.ods**, **.csv**, **.tsv** e **.txt** criam cópias, como descrito acima.

O JeffCrunch salva somente o que você alterou. Todo o resto do arquivo, como as outras planilhas, os gráficos, as imagens, as macros e as configurações que o JeffCrunch não mostra, é copiado como estava, byte por byte. Em uma planilha que você alterou, somente as células alteradas são gravadas novamente, junto com as fórmulas que dependem delas, que recebem seus novos valores. Se você não alterou nenhuma célula, o arquivo não é modificado.

Se outro programa alterou o arquivo depois que você o abriu, o JeffCrunch não o substitui sem perguntar, porque isso apagaria as alterações do outro programa. Ele pergunta se você deseja **Salvar uma Cópia…**, **Substituir** o arquivo ou **Cancelar**.

O novo arquivo é gravado primeiro ao lado do antigo e só toma o lugar dele quando está completo. Se algo der errado, você verá uma mensagem, e o arquivo que estava lá permanecerá como estava.

Se as fórmulas ainda estiverem sendo calculadas quando você salvar, o JeffCrunch espera o cálculo terminar, para que o arquivo contenha os resultados. Se você pressionar **Esc** para interromper o cálculo nesse meio-tempo, o salvamento é cancelado, e a barra de status informa isso; salve novamente quando quiser. O JeffCrunch deixa uma coisa de fora do arquivo: a lista da ordem em que um programa calculou as fórmulas pela última vez (a cadeia de cálculo). O próximo programa que abrir a pasta de trabalho a recria. Se o JeffCrunch não conseguiu calcular alguma fórmula, ou se você interrompeu o cálculo, o arquivo pede a esse programa que recalcule todas as fórmulas ao abrir a pasta de trabalho.

## Fechar com alterações

Se você fechar uma pasta de trabalho com alterações não salvas, o JeffCrunch pergunta se você deseja **Salvar**, **Não Salvar** ou **Cancelar** e continuar trabalhando. Um asterisco (*) no título da janela indica que há alterações não salvas.
