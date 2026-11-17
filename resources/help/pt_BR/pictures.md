# Adicionar e ajustar imagens
<!-- keywords: efeitos de imagem, sombra, reflexo, brilho, bordas suaves, bisel, rotação 3D, redefinir imagem e tamanho, foto, figura, inserir imagem, cortar, clip-art, página de PDF, jpg, png, svg, recolorir, texto alternativo, legenda, espaço reservado, gerenciador de elementos gráficos, imagens online, vincular ao arquivo, imagem vinculada, inserir e vincular, incorporar, imagem ausente, vincular novamente, figura, imagem, cartão, moldura -->

Uma imagem fica dentro de uma moldura. Você pode cortar a imagem e mover ou redimensionar a moldura.

## Inserir uma imagem de um arquivo

1. Clique em **Inserir > Ilustrações > Imagens**.
2. Na janela **Inserir Imagem**, escolha um arquivo. O JeffPub abre arquivos PNG, JPEG, GIF, BMP, TIFF, WebP, SVG, WMF, EMF, ICO e PDF.
3. Escolha **Inserir**, **Vincular ao Arquivo** ou **Inserir e Vincular** na caixa **Inserir como**, que fica junto da lista de arquivos e pode ser acessada com a tecla Tab. **Inserir** vem selecionada sempre que a janela é aberta. Veja "Vincular uma imagem ao seu arquivo", a seguir.
4. Clique em **Abrir**.

A imagem aparece no meio da página. Você também pode arrastar um arquivo de imagem para a página, o que sempre usa **Inserir**.

O JeffPub lê arquivos de imagem de até 256 MB. Um arquivo maior é recusado, assim como um arquivo vazio ou qualquer coisa que não seja um arquivo comum no disco, como um dispositivo ou um canal. A mensagem informa que o JeffPub não consegue lê-lo como imagem.

Se você escolher várias imagens de uma vez, elas são organizadas como miniaturas na área de rascunho, o espaço cinza ao lado da página. Arraste para a página as que quiser usar.

Se um PDF tiver mais de uma página, a janela **Inserir Página de PDF** mostra cada página. Escolha uma e clique em **OK**.

## Vincular uma imagem ao seu arquivo

**Inserir** coloca uma cópia da imagem dentro da publicação. Essa cópia vai para onde a publicação for, mas uma publicação com muitas fotos grandes também fica grande. Um vínculo mantém as fotos em seus próprios arquivos.

- **Inserir** copia a imagem para dentro da publicação. É a escolha usual.
- **Vincular ao Arquivo** guarda apenas a localização do arquivo e uma pequena visualização, de no máximo 512 pixels no lado mais longo. A imagem na página, e na impressão, no PDF ou no e-book, vem do arquivo em qualidade total. Se o arquivo for movido ou excluído, a página mostra a pequena visualização.
- **Inserir e Vincular** faz as duas coisas. A publicação guarda a imagem inteira e o vínculo. Quando você abre a publicação e o arquivo mudou, o JeffPub lê o arquivo novamente e substitui a sua cópia.

O JeffPub guarda a localização completa do arquivo e também a sua posição em relação à publicação, como "a pasta Fotos ao lado dela". Se você mover a publicação e as imagens juntas, a segunda informação prevalece e as imagens são encontradas novamente. Mantenha as pastas como estavam.

O JeffPub abre sozinho o arquivo de uma imagem vinculada somente quando ele está na pasta da publicação ou em uma pasta dentro dela. Uma imagem vinculada a um arquivo em qualquer outro lugar mostra a visualização salva, borrada, e aparece marcada como **Não atualizada**, até que você a selecione e clique em **Atualizar Vínculo** no Gerenciador de Elementos Gráficos. Isso evita que uma publicação enviada por outra pessoa acesse o seu computador. Para que as imagens carreguem sozinhas, guarde-as na pasta da publicação.

O **Gerenciador de Elementos Gráficos** mostra se cada imagem está **Incorporada**, **Vinculada**, **Ausente**, **Modificada** ou **Não atualizada**, e tem botões para atualizar um vínculo, apontá-lo para outro arquivo ou transformá-lo em uma cópia. Veja [Gerenciador de Elementos Gráficos](graphics-manager).

Ao salvar uma publicação como arquivo `.pub`, criar um arquivo Empacotar ou enviá-la por e-mail, o JeffPub coloca cada imagem vinculada inteira no arquivo, para que ele não dependa de arquivos que ficam para trás. Se um arquivo vinculado estiver ausente nesse momento, ou se a imagem estiver **Não atualizada**, entra no lugar dela a sua pequena visualização. Um arquivo `.pub` que você abre nunca tem vínculos: as imagens entram como cópias.

## Adicionar um espaço reservado para imagem

Um espaço reservado é uma moldura vazia que guarda o lugar de uma imagem. Clique em **Inserir > Ilustrações > Espaço Reservado para Imagem**, depois clique duas vezes na moldura e escolha um arquivo. A imagem preenche a moldura, e as bordas que sobram ficam ocultas. Um espaço reservado vazio não é impresso.

## Encontrar imagens gratuitas na internet

Clique em **Inserir > Ilustrações > Imagens Online**. O painel **Imagens Online** pesquisa duas bibliotecas gratuitas de imagens com licença aberta: a Openverse, que reúne muitas coleções, e o Wikimedia Commons, a biblioteca de imagens da Wikipédia.

1. Escolha uma biblioteca, digite o que procura, como "celeiro vermelho", e pressione Enter.
2. Clique em uma imagem para ver quem a criou e a sua licença. A licença é o conjunto de regras para usar a imagem. A maioria pede que você cite o autor.
3. Clique em **Inserir**. O JeffPub coloca a imagem na página e usa o título dela como texto alternativo. Com **Adicionar crédito abaixo da imagem** marcado, uma linha como "Celeiro vermelho" de Pat Lee, CC BY 2.0 aparece embaixo dela.

As palavras da sua pesquisa são enviadas à biblioteca escolhida. Verifique se a licença de cada imagem combina com o uso que você vai fazer.

Uma pesquisa mostra até 24 imagens. O JeffPub usa apenas endereços da web seguros (https) de uma biblioteca e deixa de fora a imagem cujo endereço não seja seguro. No Wikimedia Commons, uma imagem com mais de 1.920 pixels de largura é obtida como uma cópia com essa largura, e não no arquivo original enviado. Um download com mais de 50 MB, ou que demore demais, é interrompido, e o painel informa que a imagem não pôde ser baixada ou carregada.

## Cortar uma imagem

Cortar oculta as bordas de uma imagem.

1. Clique na imagem e depois em **Formato da Imagem > Cortar > Cortar**.
2. Arraste uma alça para ocultar parte da imagem. Arraste a imagem para deslizá-la dentro da moldura.
3. Clique em **Cortar** de novo, ou pressione **Esc**.

Também no grupo **Cortar**: **Proporção** corta a moldura em um formato como quadrado, 4 de largura por 3 de altura, ou 16 por 9, em torno do centro dela (a imagem continua no mesmo lugar da página). **Ajustar** mostra a imagem inteira dentro da moldura, **Preencher** preenche a moldura, **Cortar para Forma** recorta a imagem em um formato como oval, e **Limpar Corte** mostra a imagem inteira novamente.

## Mudar a aparência de uma imagem

Na guia **Formato da Imagem**, use o grupo **Ajustar**:

- **Correções**: escolha uma combinação de brilho e contraste, de menos 40 a mais 40 por cento. **Formatar Objeto…**, no final do menu, tem controles deslizantes.
- **Recolorir**: deixe a imagem em Escala de Cinza, Sépia, Desbotado, Preto e Branco ou em uma tonalidade de destaque. **Definir Cor Transparente** torna transparente a cor do canto superior esquerdo da imagem.
- **Transparência…**: esmaece a imagem inteira.
- **Redefinir Imagem**: desfaz todas as alterações desta lista, além da borda, da forma e dos efeitos da imagem. O corte também é desfeito, e a moldura mostra a imagem inteira. Clique na seta ao lado para **Redefinir Imagem e Tamanho**, que também devolve à imagem o tamanho que ela tem no arquivo.
- **Compactar Imagens…**: reduz o arquivo diminuindo o detalhe para 300, 220 ou 96 ppi (pixels por polegada). Por padrão, altera todas as imagens e exclui as áreas cortadas.
- **Converter em Formas**: transforma uma imagem SVG (um desenho feito de linhas e formas) em formas que você pode editar, recortadas onde o desenho as corta. Textos e imagens dentro dela não são incluídos.

## Estilos, bordas e efeitos de imagem

Use o grupo **Estilos de Imagem** na guia **Formato da Imagem**:

- A **galeria** mostra cerca de duas dúzias de aparências prontas: molduras, sombras, reflexos, bordas suaves, cantos arredondados e ovais, uma rotação no espaço, um brilho e biséis. Um bisel é uma borda inclinada que faz a imagem parecer elevada. Cada miniatura mostra as suas cores. Clique em uma para dar à imagem essa aparência. Um estilo define a forma, a borda e os efeitos ao mesmo tempo, e substitui os que a imagem tinha. O corte, o tamanho e as cores são mantidos.
- **Borda da Imagem** define a cor, a **Espessura** (o grossura da linha) e os **Traços** (linha contínua, pontilhada ou tracejada). **Sem Contorno** remove a borda.
- **Efeitos de Imagem** abre **Sombra**, **Reflexo**, **Brilho**, **Bordas Suaves**, **Bisel** e **Rotação 3D**. Cada um tem opções prontas. Escolha uma para adicionar o efeito. Os outros efeitos são mantidos. Escolha a primeira entrada, como **Sem Sombra**, para remover um deles. **Mais Opções de Efeitos…** abre **Formatar Objeto**.

Cada uma dessas ações pode ser desfeita com um único passo.

## Legenda, texto alternativo e troca

- **Formato da Imagem > Estilos de Imagem > Legenda** adiciona uma caixa de texto abaixo da imagem.
- O texto alternativo é uma descrição curta para pessoas que não conseguem ver a imagem, como usuários de leitores de tela. Clique com o botão direito na imagem, escolha **Formatar Objeto…**, abra a guia **Texto Alternativo** e digite em **Texto alternativo**.
- **Formato da Imagem > Inserir > Alterar Imagem** coloca outro arquivo na mesma moldura. A moldura mantém o tamanho, o lugar e a aparência: a borda, a forma, os efeitos e as alterações de cor. Se a nova imagem tiver a mesma forma da antiga, o corte também é mantido. Se tiver outra forma, ela preenche a moldura.
- Para trocar duas imagens, arraste uma sobre a outra. Quando a dica mostrar **Solte para trocar as imagens**, solte: as imagens trocam de moldura, e a moldura que você arrastou volta ao lugar original. Ou mantenha Shift pressionado, clique nas duas e clique em **Trocar**. Mantenha Ctrl pressionado enquanto arrasta para copiar uma imagem. Uma cópia solta sobre outra imagem fica onde você a colocou, e nada é trocado.

## Ver todas as imagens

Clique em **Exibir > Mostrar > Gerenciador de Elementos Gráficos** para listar todas as imagens no painel **Gerenciador de Elementos Gráficos**. Selecione uma e clique em **Ir para** para ir até ela, em **Salvar Como…** para salvar uma cópia, ou em **Substituir…** para escolher outro arquivo. Para imagens vinculadas, há também **Atualizar Vínculo**, **Alterar Vínculo…** e **Incorporar Imagem**. Veja [Gerenciador de Elementos Gráficos](graphics-manager).

## Veja também

- [Gerenciador de Elementos Gráficos](graphics-manager)
- [Desenhar formas e linhas](shapes)
- [Mover, alinhar, agrupar e organizar objetos em camadas](arrange)
- [A guia Formato da Imagem](tab-picture-format)
