# Separador Reprodução

<!-- keywords: separador reprodução, friso, filme, vídeo, som, áudio, reproduzir, pausa, pré-visualizar, cortar, legendas, inserir legendas, remover legendas, subtítulos, webvtt, vtt, volume, sem som, iniciar, na sequência de cliques, automaticamente, ao clicar, reproduzir em ecrã inteiro, ocultar quando não estiver a reproduzir, repetir até parar, recuar após a reprodução, reproduzir entre diapositivos, continuar a reproduzir, texto alternativo, alinhar, altura, largura, tamanho, ferramentas de multimédia -->

O separador **Reprodução** define como um filme ou um som é reproduzido e acrescenta legendas a um filme. Só aparece enquanto está selecionado um filme ou um som, e desaparece quando seleciona outra coisa.

- **Pré-visualizar** tem **Reproduzir**. Reproduz o filme ou o som no diapositivo para que possa ver e ouvir. Enquanto está a reproduzir, o botão diz **Pausa**.
- **Edição** tem **Cortar**. Abre uma caixa com **Hora de início** e **Hora de fim**, em segundos. O filme ou o som é reproduzido apenas entre a hora de início e a hora de fim. O restante do ficheiro mantém-se dentro da apresentação. Para voltar a reproduzir até ao fim do ficheiro, defina a hora de fim como a duração total.
  - **Inserir Legendas** acrescenta legendas a um filme a partir de um ficheiro WebVTT (um ficheiro de texto que termina em `.vtt`). Pede o ficheiro e depois o idioma das legendas, como um código curto, por exemplo `en`, `es` ou `pt-BR`. As legendas acompanham um filme que esteja incorporado na apresentação; para um filme que está apenas ligado a partir do exterior, o botão fica esbatido. Um filme mantém um conjunto de legendas por cada idioma: inserir um segundo ficheiro para o mesmo idioma substitui o primeiro. Um ficheiro que não seja WebVTT, que não tenha legendas ou que seja demasiado grande (o máximo é 4 MB, 20 000 legendas e 2000 carateres numa só legenda) é recusado com uma mensagem, e nada é alterado.
  - **Remover Legendas** retira as legendas do filme. Se o filme tiver legendas em mais de um idioma, pergunta qual remover ou se devem ser removidas todas.
- **Opções de Multimédia** indica como o filme ou o som é reproduzido na apresentação de diapositivos:
  - **Volume** é **Baixo**, **Médio**, **Alto** ou **Sem som**.
  - **Iniciar** indica o que o inicia. **Na Sequência de Cliques** reproduz o filme ao clicar, junto com as animações do diapositivo. **Automaticamente** reproduz o filme por si quando chega a sua vez. **Ao Clicar** reproduz o filme ou o som quando clica no filme ou no som.
  - **Reproduzir em Ecrã Inteiro** faz com que um filme ocupe o ecrã enquanto é reproduzido. Funciona apenas para filmes.
  - **Ocultar Quando Não Estiver a Reproduzir** oculta a imagem até o filme ou o som começar a ser reproduzido, e volta a ocultá-la depois de parar. A imagem do altifalante de um som fica oculta durante todo o tempo.
  - **Repetir Até Parar** reproduz de novo sempre que termina, até ser parado ou até o diapositivo mudar.
  - **Recuar Após a Reprodução** faz com que um filme volte à primeira imagem quando termina. Sem esta opção, o filme fica na última imagem.
  - **Reproduzir Entre Diapositivos** (para um som; um filme é apresentado apenas no seu próprio diapositivo) mantém o som a tocar na apresentação de diapositivos quando o diapositivo muda, até terminar ou ser parado, até ao fim da apresentação. Um ficheiro de outro programa pode indicar em quantos diapositivos o som é reproduzido: esse número mantém-se enquanto a caixa estiver marcada, e o som pára após esse número de diapositivos, contados a partir do diapositivo em que começou (voltar atrás e à frente entre eles não conta). Desmarcada, o som pára com o seu diapositivo; marcada de novo, continua até ao fim da apresentação. Com **Repetir Até Parar** recomeça sempre que termina. Ctrl+Z anula a alteração, e um ficheiro .pptx guarda-a.

As legendas aparecem sobre a parte inferior do filme, em texto branco sobre uma faixa escura, enquanto o filme é reproduzido na apresentação de diapositivos e no editor. Estão sincronizadas com o filme, e o seu tamanho acompanha o tamanho do filme. Se um filme tiver mais de um idioma, aparece o primeiro. As legendas são guardadas com a apresentação, como parte do ficheiro `.pptx`, pelo que voltam a aparecer quando a abre novamente. Um ficheiro de legendas noutro formato (que não seja WebVTT) é mantido tal como está e volta a ser guardado sem alterações, mas não é apresentado. Depois de um filme ter legendas, **Verificar Acessibilidade** deixa de o indicar.

Os grupos **Acessibilidade**, **Organizar** e **Tamanho** são os mesmos do separador Formato da Imagem: **Texto Alternativo**, **Alinhar** e as caixas **Altura** e **Largura**, para o filme ou o som selecionado.

Cada alteração é um passo. **Ctrl+Z** anula-a.

Um som ou filme que é reproduzido entre diapositivos, como alguns ficheiros definem, continua a ser reproduzido pelo número de diapositivos que o ficheiro indica. O JeffDeck reproduz desse modo na apresentação de diapositivos e mantém a definição quando guarda.

Consulte [Formas, imagens, tabelas e Texto Artístico](objects) e [Reproduzir a apresentação de diapositivos e usar a Vista de Apresentador](show).

## Relacionados

- [Formas, imagens, tabelas e Texto Artístico](objects)
- [Separador Inserir](tab-insert)
- [Reproduzir a apresentação de diapositivos e usar a Vista de Apresentador](show)
