# Abrir e guardar ficheiros OpenDocument
<!-- keywords: odt, fodt, ott, opendocument, open document, documento aberto, abrir, guardar, guardar como, formato, tipo de ficheiro, outros programas, trocar documentos, partilhar, compatibilidade, controlo de alterações, alterações controladas, omitido, modelo -->

O JeffWrite abre e guarda texto OpenDocument, o tipo de ficheiro cujo nome termina em **.odt**. Muitos programas gratuitos leem e escrevem este formato, pelo que é uma boa forma de trocar um documento com alguém que não utiliza o JeffWrite.

## Abrir um ficheiro OpenDocument

Prima **Ctrl+O**, ou clique em **Ficheiro > Abrir**, e escolha um ficheiro:

- **.odt** é um documento.
- **.ott** é um modelo. Abre como um novo documento criado a partir dele, pelo que o modelo fica inalterado.
- **.fodt** é o mesmo tipo de documento guardado num único ficheiro XML simples. O JeffWrite não escreve ficheiros .fodt: **Guardar** num destes ficheiros pede um novo nome e guarda-o como ficheiro .odt.

O JeffWrite identifica o tipo de ficheiro pelo respetivo conteúdo e não pelo nome, pelo que um ficheiro com a extensão errada também abre. Também pode indicar o nome de um ficheiro ao iniciar o JeffWrite ou arrastá-lo do gestor de ficheiros para o programa, tal como faz com um ficheiro .docx.

O JeffWrite lê o texto e a respetiva formatação, os títulos, os estilos e os estilos em que se baseiam, as listas com marcas e numeradas, as tabelas (com células unidas, larguras de coluna, limites, sombreado e tabelas dentro de tabelas), as secções e colunas, o tamanho, as margens e a orientação da página, os cabeçalhos e rodapés (primeira página, esquerda e direita), as notas de rodapé e notas de fim, os campos (números de página, número total de páginas, título, autor, nome do ficheiro, referências cruzadas e números sequenciais), os marcadores, as ligações, o índice, as imagens, as caixas de texto, as posições de tabulação, os comentários e as alterações controladas, que são apresentadas como marcas que pode aceitar ou rejeitar.

Há coisas que o JeffWrite ainda não consegue mostrar: formas desenhadas que não contêm texto, fórmulas, gráficos e outros objetos inseridos no texto. Se um ficheiro tiver algum destes elementos, o JeffWrite indica quais ao abri-lo. Nesse caso, não guarda por cima desse ficheiro: **Guardar** pede um novo nome, para que o original mantenha o que a cópia perdeu. Os documentos protegidos por palavra-passe não são abertos. Abra-os no programa que os criou e guarde uma cópia sem palavra-passe.

## Guardar um ficheiro OpenDocument

Clique em **Ficheiro > Guardar Como** e escolha **Texto OpenDocument (*.odt)** na lista de tipos de ficheiro. A partir daí, **Ctrl+S** guarda novamente no mesmo formato.

O ficheiro contém tudo o que está indicado acima, incluindo os comentários e as alterações controladas: texto adicionado e eliminado, formatação alterada e quebras de parágrafo adicionadas ou eliminadas. Para algumas coisas o formato não tem lugar, e o JeffWrite indica quais são depois de guardar: são guardadas tal como estão no momento ou omitidas. Trata-se das alterações controladas em tabelas, colunas e configuração da página, e das alterações controladas em cabeçalhos, rodapés e notas (guardadas tal como estão no momento); dos campos de formulário (guardados como o texto que mostram); das equações (guardadas como texto simples); e dos desenhos, formas, gráficos e objetos incorporados que não é possível manter. Se voltar a guardar depois de uma alteração que omita algo novo, o JeffWrite também o informa, sem repetir o que já lhe indicou.

Os outros programas que leem OpenDocument abrem o ficheiro sem pedir para o reparar.

## Tópicos relacionados

- [Abrir, guardar, imprimir e exportar](open-save)
- [Controlar alterações e aceitá-las ou rejeitá-las](track-changes)
- [Bem-vindo ao JeffWrite](start)
