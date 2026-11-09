# Verificar a sua estrutura

<!-- keywords: verificador de estrutura, verificar, erros, problemas, baixa resolução, excesso, texto não cabe, caixa de texto vazia, fora da página, pré-impressão, verificação final, prova, imagem ligada, tipo de letra em falta -->

O **Verificador de Estrutura** analisa a sua publicação à procura de situações que costumam correr mal. Encontra texto que não cabe, caixas vazias, objetos que ultrapassam a página e imagens que vão imprimir mal. Execute-o antes de imprimir ou de enviar um ficheiro para uma gráfica.

## Executar o Verificador de Estrutura

Clique em **Revisão > Verificar > Executar Verificador de Estrutura**. O painel **Verificador de Estrutura** abre-se à direita. Também pode clicar em **Ficheiro > Informações** e, depois, em **Executar Verificador de Estrutura**.

O painel tem duas caixas de verificação, ambas ativadas inicialmente:

- **Executar verificações gerais de estrutura** procura problemas de esquema.
- **Executar verificações finais de publicação** procura aspetos importantes na impressão.

Deixe ambas ativadas para ver todos os tipos de problemas. A lista atualiza-se à medida que trabalha. Se não houver nada para corrigir, aparece a mensagem "Não foram encontrados problemas."

## Corrigir um problema

1. Clique num item da lista em **Selecione um item para corrigir**.
2. Clique em **Ir Para o Item**. Ou faça duplo clique no item.

O JeffPub vai para a página e seleciona o objeto. Corrija-o e o item sai da lista.

## O que é encontrado

| Mensagem | O que significa | O que fazer |
|---|---|---|
| A página não tem conteúdo | A página está vazia. | Acrescente algo ou elimine a página. |
| O objeto está parcialmente fora da página | Parte de um objeto está fora da página. | Mova-o ou deixe-o se for uma sangria. |
| A caixa de texto está vazia | Uma caixa de texto não tem texto. | Escreva nela ou elimine-a. |
| Artigo com texto em excesso | Um artigo é o texto que passa por caixas ligadas. Este tem mais texto do que cabe. | Aumente uma caixa, corte texto ou [ligue outra caixa](linked-text). |
| O texto foi muito reduzido para caber | Reduzir o texto para caber na caixa deixou-o com menos de metade do tamanho. | Aumente a caixa ou use menos texto. |
| O marcador de posição de imagem está vazio | Um marco de imagem não tem imagem. | Clique nele para acrescentar uma imagem ou elimine-o. |
| A imagem não está dimensionada proporcionalmente | A imagem está esticada ou comprimida. | Redimensione-a para repor a sua forma. |
| A imagem está ligada, não incorporada | A imagem está associada a um ficheiro e apenas uma pequena pré-visualização fica guardada na publicação. Uma imagem inserida com **Inserir e Ligar** é guardada completa e não aparece na lista. | Guarde o ficheiro original junto da sua publicação ou clique em **Incorporar Imagem** no [Gestor de Gráficos](graphics-manager). |
| A imagem ligada está em falta | O ficheiro para o qual a imagem ligada aponta já não existe, pelo que a página mostra apenas a sua pequena pré-visualização, e é isso que seria impresso. | No Gestor de Gráficos, clique em **Alterar Ligação…** para encontrar o ficheiro, ou repô-lo e clicar em **Atualizar Ligação**. |
| A imagem ligada mostra apenas a pré-visualização até Atualizar Ligação | A imagem está ligada a um ficheiro fora da pasta da publicação, que o JeffPub não abre por si só. A página mostra a pequena pré-visualização, e é isso que seria impresso. | No Gestor de Gráficos, selecione a imagem e clique em **Atualizar Ligação**. |
| A imagem tem baixa resolução | A imagem tem menos de 150 ppi (pixels por polegada) no tamanho impresso. | Use uma imagem maior ou imprima-a mais pequena. |
| O objeto tem transparência ou efeitos (pode ser impresso de forma diferente) | As áreas transparentes e os efeitos podem ter outro aspeto no papel. | Imprima uma página de teste. |
| O tipo de letra "nome" não está instalado | Um tipo de letra do seu texto não está neste computador, pelo que outro o substitui. | Instale o tipo de letra ou escolha outro. |

Cada mensagem termina com o número da página, como "(Página 2)".

## Antes de um ficheiro para uma gráfica

Ao criar um ficheiro PDF/X, o JeffPub avisa-o sobre imagens de baixa resolução e tipos de letra em falta, e oferece-se para abrir o Verificador de Estrutura. Consulte [Ficheiros PDF e gráficas](pdf).

## Consulte também

- [Ficheiros PDF e gráficas](pdf)
- [Imprimir uma publicação](printing)
- [Fazer fluir texto entre caixas de texto](linked-text)
- [Adicionar e ajustar imagens](pictures)
- [Gestor de Gráficos](graphics-manager)
