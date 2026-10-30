# Abrir e salvar pastas de trabalho
<!-- keywords: novo, nova, em branco, pasta1, book1, sem título, abrir, salvar, salvar como, arquivo, pasta de trabalho, planilha, xlsx, xlsm, xls, ods, csv, tsv, arquivo de texto, delimitado, separado por vírgulas, importar, exportar, recentes, fixado, fixar, arrastar, soltar, carregamento, carregando, grande, macros, não salvo, alterações, fechar, cópia, cadeia de cálculo -->

Quando o JeffCrunch é iniciado sem nenhum arquivo para abrir, a janela mostra uma nova pasta de trabalho vazia, com uma planilha, chamada Pasta1. Cada nova pasta de trabalho recebe o número seguinte (Pasta2, Pasta3 e assim por diante), e um número não é usado novamente depois que a janela dele é fechada. Para começar outra, pressione **{key:New}** ou clique em **Arquivo**, depois em **Novo**, e escolha **Pasta de trabalho em branco**; ela abre em uma janela própria. Uma nova pasta de trabalho que você não alterou dá lugar ao arquivo que você abrir na janela dela; uma que você alterou permanece, e o arquivo abre em outra janela. **Salvar** em uma nova pasta de trabalho pede um nome, como **Salvar Como**, e a salva como arquivo .xlsx. Fechar uma nova pasta de trabalho que você não alterou não exibe nenhuma pergunta.

Para abrir uma pasta de trabalho, pressione **{key:Open}** ou clique em **Arquivo** e depois em **Abrir**, e escolha **Procurar**. Você também pode arrastar uma pasta de trabalho do gerenciador de arquivos para a janela. Uma pasta de trabalho que já está aberta em uma janela não é aberta uma segunda vez: essa janela vem para a frente.

O JeffCrunch abre pastas de trabalho **.xlsx** e **.xlsm** (as que contêm macros). Ele não executa macros e as mantém intactas.

## Outros tipos de arquivo

O JeffCrunch também abre pastas de trabalho **.xls** mais antigas, planilhas OpenDocument (**.ods** e o formato plano **.fods**) e arquivos de texto de valores (**.csv**, **.tsv** e **.txt**). Uma pasta de trabalho aberta a partir de um deles é salva como um novo arquivo **.xlsx**: quando você pressiona **{key:Save}**, o JeffCrunch avisa e pede um nome, e o arquivo que você abriu nunca é alterado.

Uma planilha OpenDocument é lida com suas planilhas, valores, fórmulas (com os resultados guardados no arquivo), formatos de número, fontes, preenchimentos, bordas, células mescladas, larguras de coluna e alturas de linha, linhas e colunas ocultas, painéis congelados e nomes. Gráficos, imagens e outros desenhos, comentários, tabelas dinâmicas, macros, formatações condicionais e validação de dados ainda não são lidos; uma mensagem na parte superior informa quais deles o arquivo tem, e uma cópia salva a partir daqui não os terá. Uma planilha protegida por senha não é aberta: abra-a no programa que a criou e salve uma cópia sem a senha.

Um arquivo de texto é uma planilha. O JeffCrunch descobre como os valores estão separados (por vírgulas, ponto e vírgula, tabulações ou barras verticais) e como os caracteres estão codificados (UTF-8 ou UTF-16, ou então a página de código do seu idioma), e lê cada valor como você o digitaria no seu idioma: um número como 1.234,5 em português, uma porcentagem, um valor monetário, uma data com ano (como 15/03/2024, ou 3/15/2024 em inglês), uma hora, VERDADEIRO ou FALSO (no seu idioma, como WAHR e FALSCH em alemão, e em inglês, TRUE e FALSE). Um valor que começa com sinal de igual é uma fórmula. Dígitos que começam com zero (como 007) e números com mais de 15 dígitos ficam como texto, para que um CEP ou um número de conta não seja alterado, assim como um dia e um mês sem ano, em qualquer ordem e em qualquer idioma (como 3/4, 12.5, 1-2, 1º de março ou 25 dez), porque o ano teria de ser inventado, e textos como o nome do gene MARCH1. Uma planilha comporta 1.048.576 linhas e 16.384 colunas; se o arquivo tiver mais, o JeffCrunch abre o que couber e avisa você.

Quando você abre um arquivo de texto, uma janela mostra primeiro as primeiras linhas e pergunta como ele deve ser lido: o que separa os campos (qualquer combinação de vírgula, ponto e vírgula, tabulação, espaço, barra ou um caractere que você digitar, com separadores consecutivos considerados como um só, se desejar), o que delimita um campo que contém um separador, a codificação de texto e a linha em que começar. Acima de cada coluna, você escolhe como ela é lida: **Como digitado**, **Texto** (para manter como está um código de peça como 007 ou um número de conta longo), uma **Data** na ordem que você indicar, ou **Ignorar**. A visualização acompanha cada alteração e mostra no máximo as primeiras 200 linhas e 100 colunas, mas o arquivo inteiro é aberto. **Arquivo > Abrir > Abrir um arquivo de texto** faz a pergunta mesmo quando o nome do arquivo é incomum.

Para guardar apenas os valores da planilha que você está vendo, clique em **Arquivo**, depois em **Salvar Como**, e escolha o tipo de arquivo **.csv**. O JeffCrunch grava os valores como as células os mostram, em UTF-8, com ponto e vírgula entre eles quando o seu idioma usa vírgula nos números e com vírgula nos demais casos, e com o texto entre aspas quando necessário. As outras planilhas, as fórmulas, os formatos e as configurações não ficam no arquivo .csv, e a sua pasta de trabalho não é alterada ao gravá-lo.

## Pastas de trabalho grandes

O JeffCrunch lê a pasta de trabalho em segundo plano. As primeiras linhas da planilha que estava aberta quando a pasta de trabalho foi salva aparecem quase imediatamente, mesmo quando a pasta de trabalho é muito grande. A barra de status, no canto inferior esquerdo, informa o que ele está fazendo: **Abrindo**, **Lendo** uma planilha com a contagem de células lidas até o momento, **Calculando** com uma porcentagem e, por fim, **Pronto**.

Você pode rolar, selecionar e copiar enquanto ele lê. As guias das outras planilhas são ativadas quando elas são lidas. Uma pasta de trabalho danificada é aberta até onde puder ser lida, e uma mensagem informa o que ficou de fora.

## Pastas de trabalho recentes

A página **Abrir** da tela Arquivo lista as pastas de trabalho que você abriu recentemente. Clique com o botão direito em uma delas para **fixá-la** no topo da lista ou removê-la da lista.

## Salvar

Pressione **{key:Save}** ou clique em **Arquivo** e depois em **Salvar** para salvar as alterações na pasta de trabalho que você abriu. Para manter o original e criar um novo arquivo, clique em **Arquivo** e depois em **Salvar Como**, e escolha um nome. Uma pasta de trabalho com macros é salva como arquivo **.xlsm**, e uma sem macros, como arquivo **.xlsx**.

O JeffCrunch salva apenas o que você alterou. Todo o resto do arquivo, como as outras planilhas, os gráficos, as imagens, as macros e as configurações que o JeffCrunch não mostra, é copiado exatamente como estava, byte a byte. Em uma planilha alterada, apenas as células que você alterou são gravadas novamente, junto com as fórmulas que dependem delas, que recebem seus novos valores. Se você não alterou nenhuma célula, o arquivo não é modificado.

Se outro programa alterou o arquivo depois que você o abriu, o JeffCrunch não o substitui sem perguntar, porque isso faria perder as alterações do outro programa. Ele pergunta se você deseja **Salvar uma Cópia**, **Substituir** o arquivo ou **Cancelar**.

O novo arquivo é gravado primeiro ao lado do antigo e só toma o lugar dele quando está completo. Se algo der errado, você verá uma mensagem, e o arquivo que estava lá permanecerá como estava.

Se as fórmulas ainda estiverem sendo calculadas quando você salvar, o JeffCrunch espera o cálculo terminar, para que o arquivo contenha os resultados. Se você pressionar **Esc** para interromper o cálculo nesse meio-tempo, o salvamento será cancelado, e a barra de status informará isso; salve novamente quando quiser. O JeffCrunch deixa uma coisa fora do arquivo: a lista da ordem em que um programa calculou as fórmulas pela última vez (a cadeia de cálculo). O próximo programa que abrir a pasta de trabalho a recria. Se o JeffCrunch não conseguiu calcular alguma fórmula, ou se você interrompeu o cálculo, o arquivo pede a esse programa que recalcule todas as fórmulas ao abrir a pasta de trabalho.

## Fechar com alterações

Se você fechar uma pasta de trabalho com alterações não salvas, o JeffCrunch perguntará se você deseja **Salvar** as alterações, **Não Salvar** ou **Cancelar** e continuar trabalhando. Um asterisco (*) no título da janela indica que há alterações não salvas.
