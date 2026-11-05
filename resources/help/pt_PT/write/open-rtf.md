# Abrir ficheiros RTF
<!-- keywords: rtf, texto formatado, formato de texto formatado, abrir rtf, .rtf, documentos antigos, importar, abrir, converter -->

**RTF** (Rich Text Format) é um tipo de ficheiro de documento que quase todos os processadores de texto conseguem ler e escrever. O JeffWrite abre ficheiros **.rtf** e mantém o seu texto e o seu aspeto.

## Abrir um ficheiro RTF

Prima **Ctrl+O**, ou clique em **Ficheiro > Abrir** e clique em **Procurar**. Escolha **Formato RTF (*.rtf)** na lista de tipos de ficheiro para ver apenas estes ficheiros, ou escolha o ficheiro na lista de todos os documentos. Também pode arrastar um ficheiro .rtf para uma janela do JeffWrite, ou indicar o nome do ficheiro quando iniciar o JeffWrite a partir de uma linha de comandos.

O JeffWrite reconhece um ficheiro RTF pelo que tem dentro, e não pelo nome, pelo que um ficheiro com outra extensão continua a abrir-se se for, de facto, um ficheiro RTF.

## O que é transferido

O JeffWrite lê estas partes de um ficheiro RTF:

- Texto em qualquer idioma, com os respetivos tipos de letra, tamanhos, cores, negrito, itálico, sublinhado, rasurado, superior à linha, inferior à linha e realce.
- Definições de parágrafo: alinhamento, avanços, espaçamento, posições de tabulação, limites, sombreado e as definições de manter com o seguinte e manter linhas juntas.
- Estilos, listas numeradas e listas com marcas.
- Tabelas, incluindo tabelas dentro de tabelas, limites, sombreado, larguras de colunas e células unidas.
- Tamanho da página, margens, colunas, orientação da página, secções, cabeçalhos e rodapés.
- Notas de rodapé, notas de fim, campos de número de página, outros campos e marcadores.
- Alterações registadas: texto acrescentado, texto eliminado, quebras de parágrafo acrescentadas ou eliminadas e alterações de aspeto. Uma **alteração registada** é uma edição que um revisor assinalou. Consulte [Controlar alterações, e aceitá-las ou rejeitá-las](track-changes).
- Imagens nos formatos PNG, JPEG, EMF e WMF, e quebras de página e de coluna.

Se parte do ficheiro não puder ser lida, o JeffWrite indica o que deixou de fora. Alguns ficheiros RTF contêm elementos que um documento no JeffWrite ainda não suporta, como desenhos ou equações. Esses elementos são deixados de fora, mantendo-se o texto à sua volta.

## Guardar as alterações

Pode guardar um documento como ficheiro .rtf: escolha **Formato RTF (*.rtf)** em **Guardar Como**, ou prima **Ctrl+S** num documento que tenha vindo de um ficheiro .rtf. O ficheiro mantém o texto e o seu aspeto, os estilos, as listas, as tabelas, a configuração da página, os cabeçalhos e rodapés, as ligações e as imagens (PNG, JPEG e EMF). Não mantém notas de rodapé, notas de fim, comentários, alterações registadas, equações, formas nem caixas de texto. O JeffWrite indica o que deixou de fora ao guardar.

## Consulte também

- [Abrir, guardar, imprimir e exportar](open-save)
- [Bem-vindo ao JeffWrite](start)
