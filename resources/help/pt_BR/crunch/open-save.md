# Abrir e salvar pastas de trabalho
<!-- keywords: novo, nova, em branco, pasta1, sem título, abrir, salvar, salvar como, arquivo, pasta de trabalho, planilha, xlsx, xlsm, xls, ods, csv, tsv, arquivo de texto, delimitado, importar, exportar, recentes, fixado, arrastar, soltar, carregando, carregamento, grande, macros, não salvo, alterações, fechar, cópia, cadeia de cálculo, modelo, xltx, xltm, não abre, não consigo abrir, danificado, corrompido, criptografado, senha -->

Quando o JeffCrunch é iniciado sem nenhum arquivo para abrir, a janela contém uma nova pasta de trabalho vazia com uma planilha, chamada Pasta1. Cada nova pasta de trabalho recebe o número seguinte (Pasta2, Pasta3 e assim por diante), e um número não é usado novamente depois que a janela dele é fechada. Para começar outra, pressione **{key:New}**, ou clique em **Arquivo**, em **Novo** e escolha **Pasta de trabalho em branco**; ela é aberta em uma janela própria. Uma nova pasta de trabalho que você não alterou dá lugar ao arquivo quando você abre um arquivo na janela dela; uma que você alterou permanece, e o arquivo é aberto em outra janela. **Salvar** em uma nova pasta de trabalho pede um nome, como **Salvar como**, e a salva como arquivo .xlsx. Fechar uma nova pasta de trabalho que você não alterou não faz nenhuma pergunta.

Para abrir uma pasta de trabalho, pressione **{key:Open}**, ou clique em **Arquivo** e em **Abrir**, e escolha **Procurar**. Você também pode arrastar uma pasta de trabalho do gerenciador de arquivos para a janela. Uma pasta de trabalho que já está aberta em uma janela não é aberta uma segunda vez: essa janela passa para a frente.

O JeffCrunch abre pastas de trabalho **.xlsx** e **.xlsm** (as que contêm macros), e os modelos **.xltx** e **.xltm** (um modelo que contém macros). Ele não executa macros e as mantém intactas: o projeto de macro, sua assinatura e as partes que fazem referência a elas são gravados de volta exatamente como estavam. Um modelo é salvo como modelo, e uma pasta de trabalho com macros como pasta de trabalho com macros, porque o tipo faz parte do arquivo.

## Outros tipos de arquivo

O JeffCrunch também abre pastas de trabalho **.xls** mais antigas, planilhas OpenDocument (**.ods** e o **.fods** plano) e arquivos de texto de valores (**.csv**, **.tsv** e **.txt**). Uma pasta de trabalho aberta de um desses arquivos é salva como um novo arquivo **.xlsx**: quando você pressiona **{key:Save}**, o JeffCrunch avisa e pede um nome, e o arquivo que você abriu nunca é alterado.

Uma planilha OpenDocument é lida com suas planilhas, valores, fórmulas (com os resultados que o arquivo guardou), formatos de número, fontes, preenchimentos, bordas, células mescladas, larguras de coluna e alturas de linha, linhas e colunas ocultas, painéis congelados e nomes. Gráficos, imagens e outros desenhos, comentários, tabelas dinâmicas, macros, formatações condicionais e validação de dados ainda não são lidos; uma mensagem na parte superior informa quais deles o arquivo tem, e uma cópia salva a partir daqui não os terá. Uma planilha protegida por senha não é aberta: abra-a no programa que a criou e salve uma cópia sem a senha.

Um arquivo de texto é uma planilha. O JeffCrunch descobre como os valores estão separados (por vírgulas, pontos e vírgulas, tabulações ou barras verticais) e como as letras estão codificadas (UTF-8 ou UTF-16, ou então a página de código do seu idioma), e lê cada valor como você o digitaria no seu idioma: um número como 1.234,5 em alemão, uma porcentagem, um valor monetário, uma data com o ano (como 3/15/2024, ou 15.03.2024 em alemão), uma hora, TRUE ou FALSE (no seu idioma, como WAHR e FALSCH em alemão, e em inglês). Um valor que começa com um sinal de igual é uma fórmula. Dígitos que começam com zero (como 007) e números com mais de 15 dígitos permanecem como texto, para que um CEP ou um número de conta não seja alterado; o mesmo vale para um dia e um mês sem ano, em qualquer ordem e em qualquer idioma (como 3/4, 12.5, 1-2, March 1 ou Dec 25), porque o ano teria de ser inventado, e para textos como o nome de gene MARCH1. Uma planilha comporta 1.048.576 linhas e 16.384 colunas; se o arquivo tiver mais, o JeffCrunch abre o que couber e avisa você.

Quando você abre um arquivo de texto, uma janela mostra primeiro as primeiras linhas e pergunta como ele deve ser lido: o que separa os campos (qualquer combinação de vírgula, ponto e vírgula, tabulação, espaço, barra ou um caractere que você digitar, com separadores consecutivos considerados como um só, se quiser), o que delimita um campo que contém um separador, a codificação de texto e a linha em que começar. Acima de cada coluna, você escolhe como ela é lida: **Como digitado**, **Texto** (para manter um código de peça como 007 ou um número de conta longo como está), uma **Data** na ordem que você indicar, ou **Ignorar**. A visualização acompanha cada alteração e mostra no máximo as primeiras 200 linhas e 100 colunas, mas o arquivo inteiro é aberto. **Arquivo > Abrir > Abrir um arquivo de texto** faz essas perguntas mesmo quando o nome do arquivo é incomum.

Para manter somente os valores da planilha que você está vendo, clique em **Arquivo**, em **Salvar como** e escolha o tipo de arquivo **.csv**. O JeffCrunch grava os valores como as células os mostram, em UTF-8, com ponto e vírgula entre eles quando o seu idioma usa vírgula nos números, e vírgula nos demais casos, e com o texto entre aspas quando necessário. As outras planilhas, as fórmulas, os formatos e as configurações não ficam no arquivo .csv, e sua pasta de trabalho não é alterada ao gravá-lo. Antes de gravar o arquivo, o JeffCrunch informa o que sua pasta de trabalho tem que o arquivo não vai conter (as outras planilhas, fórmulas, imagens, gráficos, anotações e macros, por exemplo) e pergunta se você deseja **Salvar Somente Valores**, **Salvar como .xlsx** em vez disso e manter tudo, ou **Cancelar**.

## Pastas de trabalho grandes

O JeffCrunch lê uma pasta de trabalho em segundo plano. As primeiras linhas da planilha que estava aberta quando a pasta de trabalho foi salva aparecem quase imediatamente, mesmo quando a pasta de trabalho é muito grande. A barra de status, no canto inferior esquerdo, informa o que ele está fazendo: **Abrindo**, **Lendo** uma planilha com a contagem de células lidas até o momento, **Calculando** com uma porcentagem e, por fim, **Pronto**.

Você pode rolar, selecionar e copiar enquanto ele lê. As guias das outras planilhas são ativadas conforme elas são lidas. Uma pasta de trabalho danificada é aberta até onde pode ser lida, e uma mensagem informa o que ficou de fora.

## Se um arquivo não abrir

Quando o JeffCrunch não consegue abrir um arquivo, ele explica o motivo em palavras simples e mantém as pastas de trabalho abertas como estão. Ele distingue um arquivo vazio de um que não é uma pasta de trabalho (um documento PDF, uma imagem, um documento de texto ou uma apresentação, por exemplo); um pacote zip truncado ou danificado, o que geralmente significa que um download ou uma cópia foi interrompido, então copie-o novamente do original; uma pasta de trabalho criptografada com senha, que o JeffCrunch ainda não consegue abrir; e um arquivo composto que não contém nenhuma pasta de trabalho.

## Pastas de trabalho recentes

A página **Abrir** da tela Arquivo lista as pastas de trabalho que você abriu recentemente. Clique com o botão direito em uma delas para **fixá-la** no topo da lista ou removê-la da lista.

## Salvar

Pressione **{key:Save}**, ou clique em **Arquivo** e em **Salvar**, para salvar as alterações na pasta de trabalho que você abriu. Para manter o original e criar um novo arquivo, clique em **Arquivo** e em **Salvar como** e escolha um nome. Uma pasta de trabalho com macros é salva como arquivo **.xlsm**, um modelo como arquivo **.xltx** ou **.xltm**, e qualquer outra como arquivo **.xlsx**. Um nome que termine com outro tipo não é aceito, e a mensagem informa qual terminação usar.

O JeffCrunch salva somente o que você alterou. Todo o resto do arquivo, como as outras planilhas, os gráficos, as imagens, as macros e as configurações que o JeffCrunch não mostra, é copiado como estava, byte por byte. Em uma planilha que você alterou, somente as células alteradas são gravadas novamente, junto com as fórmulas que dependem delas, que recebem seus novos valores. Se você não alterou nenhuma célula, o arquivo não é modificado.

Se outro programa alterou o arquivo depois que você o abriu, o JeffCrunch não o substitui sem perguntar, porque isso faria perder as alterações do outro programa. Ele pergunta se você deseja **Salvar uma Cópia**, **Substituir** o arquivo ou **Cancelar**.

O novo arquivo é gravado primeiro ao lado do antigo e só toma o lugar dele quando está completo. Se algo der errado, você verá uma mensagem, e o arquivo que estava lá permanecerá como estava.

Se as fórmulas ainda estiverem sendo calculadas quando você salvar, o JeffCrunch espera até que terminem, para que o arquivo contenha os resultados. Se você pressionar **Esc** para interromper o cálculo nesse meio-tempo, o salvamento é cancelado, e a barra de status informa isso; salve novamente quando quiser. O JeffCrunch deixa uma coisa de fora do arquivo: a lista da ordem em que um programa calculou as fórmulas pela última vez (a cadeia de cálculo). O próximo programa que abrir a pasta de trabalho a refaz. Se o JeffCrunch não conseguiu calcular alguma fórmula, ou se você interrompeu o cálculo, o arquivo pede a esse programa que recalcule todas as fórmulas ao abrir a pasta de trabalho.

## Fechar com alterações

Se você fechar uma pasta de trabalho com alterações não salvas, o JeffCrunch pergunta se você deseja **Salvar** as alterações, **Não Salvar** ou **Cancelar** e continuar trabalhando. Um asterisco (*) no título da janela indica que há alterações não salvas.
