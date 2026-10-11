# Adicionar e ajustar imagens
<!-- keywords: foto, imagem, figura, inserir imagem, cortar, recortar, clip-art, página de PDF, jpg, png, svg, recolorir, texto alternativo, legenda, espaço reservado, gerenciador de elementos gráficos, imagens online, vincular ao arquivo, imagem vinculada, inserir e vincular, inserir, incorporar, imagem ausente, revincular -->

Uma imagem fica dentro de uma moldura. Você pode cortar a imagem e mover ou redimensionar a moldura.

## Inserir uma imagem de um arquivo

1. Clique em **Inserir > Ilustrações > Imagens**.
2. Na janela **Inserir Imagem**, escolha um arquivo. O JeffPub abre arquivos PNG, JPEG, GIF, BMP, TIFF, WebP, SVG, WMF, EMF, ICO e PDF.
3. Escolha **Inserir**, **Vincular ao Arquivo** ou **Inserir e Vincular** na caixa **Inserir como**, que fica junto à lista de arquivos e pode ser alcançada com a tecla Tab. **Inserir** vem selecionado sempre que a janela é aberta. Consulte "Vincular uma imagem ao seu arquivo", abaixo.
4. Clique em **Abrir**.

A imagem é colocada no meio da página. Você também pode arrastar um arquivo de imagem para a página, o que sempre usa **Inserir**.

O JeffPub lê arquivos de imagem de até 256 MB. Um arquivo maior é recusado, assim como um arquivo vazio ou qualquer coisa que não seja um arquivo comum no disco, como um dispositivo ou um pipe. A mensagem informa que o JeffPub não consegue lê-lo como imagem.

Se você escolher várias imagens ao mesmo tempo, elas são enfileiradas como miniaturas na área de rascunho, o espaço cinza ao lado da página. Arraste as que quiser para a página.

Se um PDF tiver mais de uma página, a janela **Inserir Página de PDF** mostrará cada página. Escolha uma e clique em **OK**.

## Vincular uma imagem ao seu arquivo

**Inserir** coloca uma cópia da imagem dentro da publicação. Essa cópia vai aonde a publicação for, mas uma publicação com muitas fotos grandes também fica grande. Um vínculo, em vez disso, mantém as fotos em seus próprios arquivos.

- **Inserir** copia a imagem para a publicação. Essa é a opção habitual.
- **Vincular ao Arquivo** guarda apenas o local do arquivo e uma pequena visualização, com no máximo 512 pixels no lado maior. A imagem na página, e numa impressão, num PDF ou num e-book, vem do arquivo em qualidade total. Se o arquivo for movido ou excluído, a página mostrará a pequena visualização.
- **Inserir e Vincular** faz as duas coisas. A publicação guarda a imagem inteira e o vínculo. Quando você abre a publicação e o arquivo foi alterado, o JeffPub o lê novamente e substitui sua cópia.

O JeffPub guarda o local completo do arquivo e também onde ele está em relação à publicação, por exemplo "a pasta Fotos ao lado dela". Se você mover a publicação e suas imagens juntas, vale o segundo, e as imagens são encontradas novamente. Mantenha as pastas como estavam.

O JeffPub só abre por conta própria o arquivo de uma imagem vinculada quando ele está na pasta da publicação ou numa pasta dentro dela. Uma imagem vinculada a um arquivo em qualquer outro lugar mostra a visualização salva, desfocada, e fica marcada como **Não atualizada**, até que você a selecione e clique em **Atualizar Vínculo** no Gerenciador de Elementos Gráficos. Isso impede que uma publicação que alguém lhe envia acesse seu computador. Para que as imagens sejam carregadas sozinhas, mantenha-as na pasta da publicação.

O **Gerenciador de Elementos Gráficos** mostra se cada imagem está **Inserida**, **Vinculada**, **Ausente**, **Modificada** ou **Não atualizada**, e tem botões para atualizar um vínculo, apontá-lo para outro arquivo ou transformá-lo em cópia. Consulte [Gerenciador de Elementos Gráficos](graphics-manager).

Quando você salva uma publicação como arquivo `.pub`, cria um arquivo com Empacotar ou a envia por email, o JeffPub coloca cada imagem vinculada inteira no arquivo, para que ele não dependa de arquivos que ficam para trás. Se um arquivo vinculado estiver ausente nesse momento, ou se a imagem estiver **Não atualizada**, a pequena visualização entra no lugar. Um arquivo `.pub` que você abre nunca tem vínculos: suas imagens entram como cópias.

## Adicionar um espaço reservado para imagem

Um espaço reservado é uma moldura vazia que guarda um lugar para uma imagem. Clique em **Inserir > Ilustrações > Espaço reservado para imagem**, clique duas vezes na moldura e escolha um arquivo. A imagem preenche a moldura, e as bordas que sobram ficam ocultas. Um espaço reservado vazio não é impresso.

## Encontrar imagens gratuitas online

Clique em **Inserir > Ilustrações > Imagens online**. O painel **Imagens online** pesquisa em duas bibliotecas gratuitas de imagens com licença aberta: Openverse, que abrange muitas coleções, e Wikimedia Commons, a biblioteca de imagens da Wikipédia.

1. Escolha uma biblioteca, digite o que procura, como "celeiro vermelho", e pressione Enter.
2. Clique em uma imagem para ver quem a criou e qual é a licença. A licença é o conjunto de regras para usar a imagem. A maioria pede que você cite o autor.
3. Clique em **Inserir**. O JeffPub coloca a imagem na página e usa o título dela como texto alternativo. Com **Adicionar crédito abaixo da imagem** marcado, uma linha como "Red barn" by Pat Lee, CC BY 2.0 é colocada abaixo dela.

Suas palavras de pesquisa são enviadas à biblioteca escolhida. Verifique se a licença de cada imagem se ajusta ao uso que você fará dela.

Uma pesquisa mostra até 24 imagens. O JeffPub usa somente endereços da Web seguros (https) de uma biblioteca e deixa de fora as imagens cujo endereço não seja seguro. Do Wikimedia Commons, uma imagem com mais de 1.920 pixels de largura vem como uma cópia com essa largura, e não como o envio original. Um download com mais de 50 MB, ou que demore demais, é interrompido, e o painel informa que não foi possível baixar ou carregar a imagem.

## Cortar uma imagem

Cortar oculta as bordas de uma imagem.

1. Clique na imagem e, em seguida, clique em **Formato da Imagem > Cortar > Cortar**.
2. Arraste uma alça para ocultar parte da imagem. Arraste a imagem para deslizá-la dentro da moldura.
3. Clique em **Cortar** novamente ou pressione **Esc**.

Também no grupo **Cortar**: **Ajustar** mostra a imagem inteira na moldura, **Preenchimento** preenche a moldura, **Cortar para Forma** recorta numa forma, como um oval, e **Limpar Corte** mostra a imagem inteira novamente.

## Alterar a aparência de uma imagem

Na guia **Formato da Imagem**, use o grupo **Ajustar**:

- **Correções**: escolha um par de brilho e contraste, de menos 40 a mais 40 por cento. **Formatar Objeto…**, na parte inferior do menu, tem controles deslizantes.
- **Recolorir**: deixe a imagem em Escala de cinza, Sépia, Desbotado, Preto e Branco ou numa tonalidade de ênfase. **Definir Cor Transparente** torna transparente a cor do canto superior esquerdo da imagem.
- **Transparência…**: esmaece a imagem inteira.
- **Redefinir Imagem**: desfaz essas alterações.
- **Compactar Imagens…**: reduz o arquivo diminuindo o detalhe para 300, 220 ou 96 ppi (pixels por polegada). Por padrão, altera todas as imagens e exclui as áreas cortadas.
- **Converter em Formas**: transforma uma imagem SVG (um desenho feito de linhas e formas) em formas que você pode editar, recortadas onde o desenho as corta. O texto e as imagens dentro dela ficam de fora.

## Legenda, texto alternativo e troca

- **Formato da Imagem > Estilos de Imagem > Legenda** adiciona uma caixa de texto abaixo da imagem.
- O texto alternativo é uma descrição curta para pessoas que não podem ver a imagem, como usuários de leitores de tela. Clique com o botão direito na imagem, escolha **Formatar Objeto…**, abra a guia **Texto Alt** e digite em **Texto alternativo**.
- **Formato da Imagem > Inserir > Alterar Imagem** coloca outro arquivo na mesma moldura.
- Para trocar duas imagens de lugar, arraste uma sobre a outra. Quando a dica disser **Solte para trocar as imagens**, solte: as imagens trocam de moldura, e a moldura que você arrastou volta para onde estava. Ou mantenha Shift pressionado, clique nas duas e clique em **Trocar**. Mantenha Ctrl pressionado enquanto arrasta para copiar uma imagem. Uma cópia solta sobre outra imagem fica onde você a colocou, e nada é trocado.

## Ver todas as imagens

Clique em **Exibir > Mostrar > Gerenciador de Elementos Gráficos** para listar todas as imagens no painel **Gerenciador de Elementos Gráficos**. Selecione uma e clique em **Ir para** para ir até ela, em **Salvar Como…** para salvar uma cópia ou em **Substituir…** para escolher outro arquivo. Para imagens vinculadas, ele também tem **Atualizar Vínculo**, **Alterar Vínculo…** e **Inserir Imagem**. Consulte [Gerenciador de Elementos Gráficos](graphics-manager).

## Relacionados

- [Gerenciador de Elementos Gráficos](graphics-manager)
- [Desenhar formas e linhas](shapes)
- [Mover, alinhar, agrupar e organizar objetos em camadas](arrange)
- [A guia Formato da Imagem](tab-picture-format)
