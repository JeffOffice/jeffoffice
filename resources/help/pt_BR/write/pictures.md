# Adicionar e alterar imagens
<!-- keywords: imagem, imagens, figura, inserir imagem, foto, fotografia, área de transferência, colar imagem, captura de tela, quebra de texto, quebra automática, alinhado com o texto, quadrado, comprimido, superior e inferior, atrás do texto, na frente do texto, texto alternativo, descrição, redimensionar, tamanho, alças, mover imagem, arrastar imagem, soltar, formato da imagem, bloquear proporção, cortar, recorte, brilho, contraste, correções, recolorir, escala de cinza, preto e branco, desbotado, borda, moldura, estilos de imagem, alterar imagem, redefinir imagem -->

Uma **imagem** pode ficar em uma linha de texto, como uma letra, ou flutuar na página com as palavras contornando-a. Este tópico mostra como inserir uma imagem em um documento, como alterar o tamanho, a posição e a forma como o texto a contorna, e como descrevê-la para pessoas que não podem vê-la.

## Inserir uma imagem

- Clique em **Inserir > Imagens**, depois em **Imagem**, escolha um arquivo e clique em **Abrir**. O JeffWrite lê arquivos PNG, JPEG, GIF, BMP, TIFF, WebP e SVG. Uma imagem que vem de um arquivo é mantida no documento como o arquivo que era (uma imagem WebP ou SVG é mantida como PNG).
- Ou clique em **Inserir > Imagens > Imagem da Área de Transferência**, ou pressione **Ctrl+V**, quando você tiver copiado uma imagem (uma captura de tela, por exemplo). A imagem fica guardada no documento, por isso continua lá quando você salva e abre o arquivo de novo.
- Ou arraste um arquivo de imagem do gerenciador de arquivos do computador para uma página. A imagem vai para o lugar onde você soltar.

A imagem vai para onde está o cursor, no lugar de qualquer coisa que você tenha selecionado. Ela recebe o tamanho pedido, mas não fica mais larga que a coluna nem mais alta que o texto da página, e suas proporções são mantidas. Depois de inserida, a imagem fica selecionada, e a guia **Formato da Imagem** aparece à direita da faixa de opções.

Uma imagem grande demais (mais de 40 milhões de pixels, um lado com mais de 30.000 pixels ou um arquivo com mais de 48 MB) não é inserida, e a barra de status informa isso. Também não é inserido um arquivo que não seja uma imagem.

## Alterar o tamanho

Clique na imagem para selecioná-la. Oito quadradinhos, as **alças**, aparecem nos cantos e nas laterais.

- Arraste uma alça de **canto** para aumentar ou diminuir a imagem mantendo as mesmas proporções.
- Arraste uma alça **lateral** para alterar apenas um dos lados.
- Ou digite um número em **Formato da Imagem > Altura** ou **Largura**. Com **Bloquear Proporção** ativado, o outro número acompanha. As imagens ficam entre 2 pontos e 2.000 pontos em cada lado.

Cada alteração equivale a uma etapa para desfazer.

## Cortar uma imagem

**Cortar** uma imagem é remover parte de suas bordas. Selecione a imagem e clique em **Formato da Imagem > Cortar**. Digite quanto remover da esquerda, do topo, da direita e da parte inferior, em porcentagem da imagem inteira. Clique em **OK**.

O quadro da imagem fica menor na mesma medida, de modo que a parte que sobra mantém o seu tamanho. Se você cortar de novo depois, começa a partir da imagem inteira, por isso sempre é possível recuperar o que foi cortado. Um número abaixo de zero deixa um espaço vazio ao redor da imagem. O corte em dois lados opostos pode somar no máximo 98 por cento.

## Alterar a aparência de uma imagem

Estes comandos estão na guia **Formato da Imagem**. Cada um equivale a uma única etapa para desfazer, e nenhum deles altera o arquivo da imagem dentro do documento.

- **Correções** deixa a imagem mais clara ou mais escura e lhe dá mais ou menos contraste. **Brilho** indica quão clara é a imagem. **Contraste** indica quão diferentes são as partes claras e as partes escuras.
- **Cor** transforma a imagem em tons de cinza (**Escala de Cinza**), em preto e branco puro (**Preto e Branco**) ou em um aspecto claro e desbotado (**Desbotado**). **Sem Recoloração** devolve as cores originais.
- **Estilos de Imagem** e **Borda da Imagem** colocam uma moldura ao redor da imagem. Escolha um estilo ou uma cor, depois uma **Espessura** (a grossura da linha) e **Traços** (linha contínua, pontilhada ou tracejada). Escolha **Sem Borda** para remover a moldura.
- **Alterar Imagem** coloca outra imagem no mesmo lugar. A nova imagem mantém a largura da anterior, a posição na página, a forma como o texto se distribui ao redor dela e o seu texto alternativo. A altura segue a forma própria da nova imagem.
- **Redefinir Imagem** remove o corte, as correções, a alteração de cor e a borda. A imagem volta à escala que tem agora.

## Distribuir o texto ao redor de uma imagem

**Formato da Imagem > Quebra de Texto** define como as palavras se distribuem ao redor da imagem. Você também pode clicar com o botão direito na imagem e escolher **Quebra de Texto**.

- **Alinhado com o Texto**: a imagem é um caractere de sua linha e se move junto com as palavras ao redor.
- **Quadrado**: as palavras contornam um retângulo que contém a imagem.
- **Comprimido**: as palavras ficam o mais perto possível da imagem que um retângulo permite (o JeffWrite trata esse modo como Quadrado).
- **Superior e Inferior**: as palavras param acima da imagem e recomeçam abaixo dela.
- **Atrás do Texto**: a imagem fica sob as palavras, que passam diretamente por cima dela.
- **Na Frente do Texto**: a imagem fica sobre as palavras, que passam diretamente por baixo dela.

## Mover uma imagem

Arraste a imagem. Uma imagem **alinhada com o texto** vai para o lugar do texto onde você soltar. Uma imagem com qualquer outra quebra vai para o ponto da página onde você soltar, e permanece ali enquanto o texto ao redor muda (ela fica junto ao parágrafo a que pertence, na página desse parágrafo).

Arrastar uma imagem alinhada com o texto não altera a quebra dela; para deixá-la flutuar, escolha uma quebra como **Quadrado**.

## Descrever uma imagem

Uma imagem que serve apenas para decorar a página, como uma linha ou um detalhe de borda, não precisa de descrição. Selecione-a e clique em **Formato da Imagem > Marcar como Decorativa** (ou clique com o botão direito nela). Leitores de tela então a ignoram. Clique de novo para remover a marcação.

O **texto alternativo** é uma descrição curta que os leitores de tela leem em voz alta e que aparece quando a imagem não pode ser exibida. Selecione a imagem e clique em **Formato da Imagem > Texto Alternativo** (ou clique com o botão direito na imagem e escolha **Texto Alternativo**), digite a descrição e clique em **OK**.

## Imagens de outros documentos

O JeffWrite mostra e mantém as imagens dos documentos que abre. Ele pode alterar o tamanho, a quebra, a posição e o texto alternativo de uma imagem que o documento armazena como imagem comum. Uma imagem armazenada em um formato que o JeffWrite não altera (um grupo de desenhos, por exemplo) é selecionada e mantida como estava; os comandos dela ficam desativados.
