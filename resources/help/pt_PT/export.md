# Guardar como imagem, página Web ou livro eletrónico
<!-- keywords: exportar, guardar como imagem, png, jpeg, gif, tiff, svg, imagem, fotografia, página Web, html, livro eletrónico, e-book, epub, esquema fixo, página a página, livro ilustrado, pack and go, impressão fotográfica, correio eletrónico, email -->

Utilize **Ficheiro > Exportar** para converter a publicação noutro tipo de ficheiro. A publicação em si não é alterada. Para PDFs, consulte [Ficheiros PDF e gráficas](pdf). Para ficheiros .pub, consulte [Trabalhar com ficheiros .pub](pub-files).

## Guardar páginas como imagens

1. Clique em **Ficheiro > Exportar** e, em seguida, em **Guardar como Imagem**.
2. Escreva um nome. Em **Guardar com o tipo**, escolha **PNG**, **JPEG**, **GIF**, **TIFF**, **Mapa de bits** ou **Desenho vetorial SVG**.
3. Clique em **Guardar**.
4. Para todos os tipos exceto SVG, escolha uma **Resolução (pontos por polegada)**: 96, 150, 300 ou 600. Mais pontos produzem uma imagem mais nítida e um ficheiro maior. 300 é uma boa opção para impressão.

Cada página fica no seu próprio ficheiro. Se a publicação tiver mais do que uma página, o JeffPub acrescenta -1, -2 e assim sucessivamente ao nome.

Um ficheiro **SVG** é um desenho composto por linhas e curvas, pelo que se mantém nítido em qualquer tamanho. O JeffPub converte as letras em contornos, para que o ficheiro tenha o mesmo aspeto num computador que não tenha os seus tipos de letra.

## Guardar como página Web

Clique em **Ficheiro > Exportar** e, em seguida, em **Guardar como Página Web**. O JeffPub cria um único ficheiro .html que abre em qualquer browser. Cada página da publicação aparece como uma imagem, pelo que as palavras não podem ser selecionadas nem alteradas. Cada imagem é acompanhada de uma versão em texto das palavras da página para leitores de ecrã, e as hiperligações que adicionou continuam a funcionar.

## Guardar como livro eletrónico

O **EPUB** é o formato padrão de livro eletrónico. Os leitores de livros eletrónicos, tablets e telemóveis abrem-no. Pode escolher um de dois tipos:

- **Texto que flui para se ajustar ao ecrã**: as palavras reorganizam-se de acordo com o ecrã e o tamanho de texto de cada leitor, como num romance. As páginas não correspondem uma a uma ao seu esquema.
- **Cada página exatamente como foi concebida**: cada página tem o aspeto que tem no JeffPub, como num livro ilustrado, revista ou banda desenhada. Os leitores ampliam para ler texto pequeno. As palavras ficam por baixo de cada página, para que os leitores as possam pesquisar e os leitores de ecrã as possam ler em voz alta.

1. Clique em **Ficheiro > Exportar** e, em seguida, em **Guardar como Livro Eletrónico (EPUB)**.
2. Escolha o tipo e verifique o **Título** e o **Autor**.
3. Para texto que flui, deixe a opção **Utilizar a primeira página como capa** selecionada ou desmarque-a. (Um livro página a página utiliza sempre a primeira página como capa.)
4. Clique em **OK**, escreva um nome de ficheiro e clique em **Guardar**.

Num livro com texto que flui, cada parágrafo com o estilo Título 1 inicia um novo capítulo. Os parágrafos Título 1, 2 e 3 formam o índice em ambos os tipos; um livro página a página indica cada título na página em que se encontra. As imagens e as tabelas mantêm-se no lugar. As notas de rodapé aparecem em janelas de contexto e as notas de fim ficam no final. Consulte [estilos](styles) e [índice, notas de rodapé e notas de fim](references).

## Pack and Go e impressões fotográficas

- **Pack and Go: Guardar para uma Gráfica Comercial** pede uma pasta. Guarda um PDF para impressão comercial (imagens em resolução total, com marcas de corte e as outras marcas da gráfica fora da página), a publicação e um pequeno ficheiro de texto com o tamanho da página e o número de páginas. Se algum dos três não puder ser guardado, por exemplo porque a pasta está cheia ou protegida, é apresentada uma mensagem a indicá-lo em vez de uma mensagem de êxito.
- **Pack and Go: Guardar para Outro Computador** cria um único ficheiro ZIP com a publicação e os tipos de letra fornecidos com o JeffPub que esta utiliza. Se não for possível guardar o ZIP, é apresentada uma mensagem a indicá-lo. As imagens ligadas aos respetivos ficheiros são guardadas por inteiro na publicação. Uma imagem ligada marcada como **Não atualizada** leva a pequena pré-visualização que apresenta, porque o JeffPub não abriu o respetivo ficheiro.
- **Guardar para Impressão Fotográfica** pede uma pasta e guarda cada página como um JPEG a 300 ppp.

## Partilhar por correio eletrónico

Clique em **Ficheiro > Partilhar**. Escolha **Enviar Página Atual por Correio Eletrónico**, **Enviar como Anexo** ou **Enviar como PDF**. O JeffPub não envia nada. Guarda um ficheiro .eml, que é uma mensagem de correio eletrónico que abre no seu programa de correio eletrónico. Aí, adicione os destinatários.

## Guardar como modelo

**Ficheiro > Exportar > Guardar como Modelo** adiciona a publicação a **Os Meus Modelos**. Consulte [Iniciar uma nova publicação](create).

## Relacionados

- [Ficheiros PDF e gráficas](pdf)
- [Imprimir uma publicação](printing)
- [Trabalhar com ficheiros .pub](pub-files)
- [Abrir e guardar publicações](open-save)
